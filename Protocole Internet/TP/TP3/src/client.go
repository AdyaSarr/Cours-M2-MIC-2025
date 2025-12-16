package main

import (
	"crypto/tls"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"log"
	"math"
	"net/http"
	"os"
	"strconv"
	"strings"
	"time"
)

const nbMessaage = 50
const chatServerURL = "https://localhost:8443/chat/"

var messageEtags = make(map[int]string) // Pour stocker le dernier etag de chaque message

// declaration de la variable qui stockera l'etat du flag
var showIDs *bool
var postMode *bool
var deleteID *int

// Configuration du client pour ignorer la verification TLS
func createClient() *http.Client {
	transport := &*http.DefaultTransport.(*http.Transport)
	transport.TLSClientConfig = &tls.Config{InsecureSkipVerify: true}
	return &http.Client{
		Transport: transport,
		Timeout:   50 * time.Second,
	}
}

func getMessagesIDs(client *http.Client) ([]int, float64, error) {
	startTime := time.Now() //on enregistre le temp de depart
	reponse, err := client.Get(chatServerURL)
	if err != nil {
		return nil, 0, fmt.Errorf("Erreur lors de la requete Get /chat/: %w", err)
	}
	defer reponse.Body.Close()

	if reponse.StatusCode != http.StatusOK {
		return nil, 0, fmt.Errorf("Erreur serveur, status: %s", reponse.Status)
	}
	//Lire le corps de la reponse
	bodyByte, err := io.ReadAll(reponse.Body)
	if err != nil {
		return nil, 0, fmt.Errorf("Erreur lors de la lecture du corps: %w", err)
	}
	endTime := time.Now()
	tempsEcoule := endTime.Sub(startTime)
	R := tempsEcoule.Seconds() * 1000 // Pour avoir le temps en milliseconde
	body := strings.TrimSpace(string(bodyByte))
	if body == "" {
		return []int{}, R, nil //aucun Id
	}
	idStrings := strings.Split(body, "\n")

	ids := make([]int, 0, len(idStrings))
	for _, s := range idStrings {
		trimmedS := strings.TrimSpace(s)
		if trimmedS == "" {
			continue
		}
		id, err := strconv.Atoi(trimmedS)
		if err != nil {
			log.Printf("Avertissement: Impossible de convertir l'ID '%s' en nombre: %v", trimmedS, err)
			continue
		}
		ids = append(ids, id)
	}

	return ids, R, nil
}

// Pour chaque message maintenant on lui attribut un ID, le contenu et le status 200 ok et 304 Not Modified
/* type FetchedMessage struct {
	ID     int
	Body   string
	Status int
} */
type FetchedMessage struct {
	ID   int       `json:"id,omitempty"`
	Time time.Time `json:"time,omitempty"`
	Body string    `json:"body"`
}

func getMessages(client *http.Client) ([]FetchedMessage, float64, error) {
	startTimeTotal := time.Now()
	allIDs, R, err := getMessagesIDs(client)
	if err != nil {
		return nil, R, fmt.Errorf("Erreur lors de la récupération des IDs: %w", err)
	}

	start := 0
	if len(allIDs) > nbMessaage {
		start = len(allIDs) - nbMessaage
	}

	idsToFetch := allIDs[start:]

	messagesTotals := make([]FetchedMessage, 0, len(idsToFetch))
	for _, id := range idsToFetch {
		messageURL := chatServerURL + strconv.Itoa(id) + ".json"

		//resp, err := client.Get(messageURL)
		//Creer la requete pour ajouter l'entete des Etag
		req, err := http.NewRequest("GET", messageURL, nil)
		if err != nil {
			log.Printf("Avertissement:Inmpossible de creer la requete d'ID %d: %v", id, err)
			continue
		}
		//Ajouter l'entete If-None-Match si Etag est connu
		etag, found := messageEtags[id]

		if found {
			req.Header.Set("If-None-Match", etag)
		}
		// Exécuter la requête
		resp, err := client.Do(req) // Utilisation de client.Do pour une requête personnalisée
		if err != nil {
			log.Printf("Avertissement: Échec GET pour ID %d: %v", id, err)
			continue
		}

		defer resp.Body.Close()

		//Gérer la réponse 304 Not Modified
		if resp.StatusCode == http.StatusNotModified { // 304
			messagesTotals = append(messagesTotals, FetchedMessage{
				ID:   id,
				Time: time.Time{},
				Body: "Contenu non modifié (304)",
			})
			continue // Ne pas lire le corps, il n'y en a pas
		}
		if resp.StatusCode != http.StatusOK { //200
			log.Printf("Avertissement: Message ID %d retourné statut %s", id, resp.Status)
			continue
		}
		jsonbody, err := io.ReadAll(resp.Body)
		if err != nil {
			log.Printf("Erreur de la lecture des données du body de l'ID %d  causé par l'erreur %v\n", err)
			continue
		}

		var data_recu FetchedMessage
		errr := json.Unmarshal(jsonbody, &data_recu)
		if errr != nil {
			log.Printf("Erreur sur la deseralisation sur l'id %d: %v\n", id, errr)
			continue
		}
		/* bodyBytes, err := io.ReadAll(resp.Body)
		if err != nil {
			log.Printf("Avertissement: Erreur lecture corps pour ID %d: %v", id, err)
			continue
		} */

		//Conversion du type any
		/* data_map, ok := data_recu.(map[string]any)
		if !ok {
			log.Printf("Erreur d'assertion pour ID %d: Le format racine n'est pas une map.", id)
			continue
		}

		contenu_body, ok := data_map["Body"].(string)
		if !ok {
			log.Printf("Erreur d'extraction du body de la map sur le json de l'ID %d\n ", id)
			continue
		} */
		//Stocker le nouvel ETag pour la prochaine vérification
		newETag := resp.Header.Get("ETag")
		if newETag != "" {
			messageEtags[id] = newETag
		}
		messagesTotals = append(messagesTotals, data_recu)
	}
	endTimeTotal := time.Now()
	tempsEcouleTotal := endTimeTotal.Sub(startTimeTotal)
	R_total := tempsEcouleTotal.Seconds() * 1000 // RTT Total en millisecondes
	return messagesTotals, R_total, nil
}

func adaptative_timeout(R float64, RTT_Est_init float64, Dev_init float64) (float64, float64, float64) {
	const alpha = 0.125 // Le parametre de lissage pour TCP on dit que c'est egale a 1/8
	const beta = 0.25   // facteur de lissage de la deviation
	//Calcule du RTT_Est
	RTT_Est_new := (1-alpha)*float64(RTT_Est_init) + alpha*float64(R)
	//Calcule de la deviation moyenne
	Dev_new := (1-beta)*Dev_init + beta*math.Abs(R-RTT_Est_new)
	X := RTT_Est_new + 4*Dev_new
	return X, RTT_Est_new, Dev_new
}

func postMessage(client *http.Client) {
	messagePost, err := io.ReadAll(os.Stdin)

	if err != nil {
		log.Fatalf("Erreur de l'ecture de l'entrée du terminal %v\n", err)
	}
	//le message lu est sous forme de []byte donc je dois le transformer vers un format qui tranforme io.Reader
	coprs_message := strings.NewReader(string(messagePost))
	requete, err := http.NewRequest("POST", chatServerURL, coprs_message)
	if err != nil {
		log.Fatalf("Erreur sur la preparation de la requete %v", err)
	}
	requete.Header.Set("Content-Type", "text/plain")
	//Envoie de la requete
	envoi_req, err := client.Do(requete)
	if err != nil {
		log.Fatalf("Erreur d'envoi de la requête: %v", err)
	}
	defer envoi_req.Body.Close()
	if envoi_req.StatusCode != http.StatusOK {
		responseBody, _ := io.ReadAll(envoi_req.Body)
		log.Fatalf("Le serveur a retourné un statut inattendu: %s. Réponse: %s", envoi_req.Status, responseBody)
	}
	idByte, err := io.ReadAll(envoi_req.Body)
	if err != nil {
		log.Fatalf("Erreur lors de la lecture de l'ID après POST: %v", err)
	}
	postedID := strings.TrimSpace(string(idByte))
	log.Printf("Message posté avec succès. Nouvel ID : %s", postedID)
}
func deleteMessage(client *http.Client, idToDelete int) {
	urlMessageDelete := chatServerURL + strconv.Itoa(idToDelete)

	requete_prepare, err := http.NewRequest("DELETE", urlMessageDelete, nil)
	if err != nil {
		log.Fatalf("Erreur sur la preparation de la requete %v", err)
	}

	requete_prepare.Header.Set("Content-Type", "text/html; charset=utf-8")
	//Envoi de la requete
	requete_envoi, err := client.Do(requete_prepare)
	if err != nil {
		log.Fatal("Erreur d'envoie de la requete")
	}
	defer requete_envoi.Body.Close()

	if requete_envoi.StatusCode == http.StatusNoContent {
		log.Printf("Message ID %d supprimé avec succès (204 No Content).", idToDelete)
	} else if requete_envoi.StatusCode == http.StatusNotFound {
		log.Printf("Message ID %d non trouvé (404 Not Found).", idToDelete)
	} else {
		log.Fatalf("Échec de la suppression pour ID %d. Statut: %s", idToDelete, requete_envoi.Status)
	}
}
func main() {
	//Definition des flag -show-ids, post
	//premier argument nom du flag
	//deuxieme valeur par defaut false
	//troisieme description
	showIDs = flag.Bool("show-ids", false, "Afficher les identifiants des messages.")
	postMode = flag.Bool("post", false, "Poster un message.")
	deleteID = flag.Int("delete", 0, "Delete le message dont l'id a ete donne")
	//Lire les arguments de ligne de commande
	flag.Parse()

	client := createClient()
	if *postMode {
		postMessage(client)
		return
	}
	if *deleteID != 0 {
		deleteMessage(client, *deleteID)
		return
	}
	_, R, err := getMessagesIDs(client)
	if err != nil {
		log.Fatal(err)
	}
	X, RTT_Est, Dev := adaptative_timeout(R, R, R/2.0)
	intervalle := time.Duration(X) * time.Millisecond // definition de la duree de l'intervalle
	//Creer un ticker qui enverra tous les 10 secondes une valeur sur le cannal 'ticker.C'
	//ticker := time.NewTicker(interalle)
	//On arrete le ticker lorsqu'on a en plus besoin
	//defer ticker.Stop()
	for {
		//<-ticker.C //On block l'execution tantqu'il ya pas de valeur sur le canal
		// S'il y'en a on execute on execute le code pour afficher les dernier message du serveur
		//ticker.Stop()
		messagesTotals, R_current, err := getMessages(client)
		if err != nil {
			log.Fatal(err)
		}
		if *showIDs { //C'est quant l'utilisateur tape -show-ids=true
			fmt.Println("Affichage des ids et du contenu")
			for _, elem := range messagesTotals {
				fmt.Printf("Liste des IDs récupérés et du contenu des requetes et le status : %v\n", elem)
			}
		} else {
			fmt.Println("Affichage du contenu des messages seul.")
			for _, elem := range messagesTotals {
				fmt.Printf("Liste du contenu des requetes : %v\n", elem.Body)
			}
		}
		time.Sleep(intervalle)
		X, RTT_Est, Dev = adaptative_timeout(R_current, RTT_Est, Dev)
		intervalle = time.Duration(X) * time.Millisecond
	}
}

/**
Exercice 1
*Reponse a la question 5:
						Le choix dépend de la priorité :

							-Si la priorité est la simplicité et l'implémentation rapide : URLs Différentes
							(votre approche actuelle) est préférable.

							-Si la priorité est le respect des standards et la flexibilité à long terme :
							Négociation de Contenu (Accept header) est préférable.
*Reponse a la question 4:
						Le décodage manuel est nécessaire lorsque le format JSON est non-typé et qu'un même
						champ est susceptible de contenir des valeurs de types
						différents (string, int, ou array) en fonction de la réponse du serveur.
						L'approche struct typée ne peut pas gérer cette ambiguïté sans code personnalisé.*/
