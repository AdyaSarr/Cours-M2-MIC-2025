package main

import (
	"crypto/tls"
	"flag"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"strconv"
	"sync"
	"time"
)

// Configuration du client pour ignorer la verification TLS
func createClient() *http.Client {
	transport := &*http.DefaultTransport.(*http.Transport)
	transport.TLSClientConfig = &tls.Config{InsecureSkipVerify: true}
	return &http.Client{
		Transport: transport,
		Timeout:   50 * time.Second,
		CheckRedirect: func(req *http.Request, via []*http.Request) error {
			return http.ErrUseLastResponse
		},
	}
}

func fullDownload(client *http.Client, uRL string) {
	requeteInit, err := client.Head(uRL)
	if err != nil {
		log.Fatalf("Erreur d'envoie de la requete HEAD code d'erreur : %v\n", err)
	}
	defer requeteInit.Body.Close()
	if requeteInit.StatusCode != http.StatusOK {
		log.Fatalf("Pour continuer afin de fauire des requetes seq je dois connaitre la taille du fichier status: %v\n", requeteInit.StatusCode)
	}
	reponse, err := client.Get(uRL)
	if err != nil {
		log.Fatalf("Erreur d'envoie de la requete GET: %v\n", err)
	}
	defer reponse.Body.Close()

	if reponse.StatusCode != http.StatusOK {
		log.Fatalf("Erreur du serveur: status: %v", reponse.StatusCode)
	}
	bodyReponse, err := io.ReadAll(reponse.Body)
	if err != nil {
		log.Fatalf("Erreur de lecture du corps de la reponse: %v", err)
	}
	fileName := "file_from_url.pdf"
	file, err := os.Create(fileName)
	if err != nil {
		log.Fatalf("Impossible de creer un fichier aa a cause de l'erreur %v\n", err)
	}
	defer func() {
		closerErr := file.Close()
		if closerErr != nil {
			log.Fatalf("Erreur lors de la fremeture du fichier a cause de l'errreur %v\n", closerErr)
		}
	}()
	nbOctet, err := file.Write(bodyReponse)
	if err != nil {
		log.Fatalf("Erreur d'ecriture de fichier telecharger a cause de %v\n", err)
	}
	fmt.Printf("Contenu ecrit avec succés dans %s et qui contient %d octes\n", fileName, nbOctet)

}
func downloadFileSequentiel(client *http.Client, uRL string, chunkSize int) {
	requeteInit, err := client.Head(uRL)
	if err != nil {
		log.Fatalf("Erreur d'envoie de la requete HEAD code d'erreur : %v\n", err)
	}
	defer requeteInit.Body.Close()
	if requeteInit.StatusCode != http.StatusOK {
		log.Fatalf("Pour continuer afin de fauire des requetes seq je dois connaitre la taille du fichier status: %v\n", requeteInit.StatusCode)
	}
	etagFirstRep := requeteInit.Header.Get("ETag")
	valLastModified := requeteInit.Header.Get("Last-Modified")
	var ifRangeValue string
	if etagFirstRep != "" {
		ifRangeValue = etagFirstRep
		log.Printf("Validateur trouvé ETag: %s\n", etagFirstRep)
	} else if valLastModified != "" {
		ifRangeValue = valLastModified
		log.Printf("Validateur trouvé Last-Modified: %s\n", valLastModified)
	} else {
		log.Printf("Aucun validateur n'a ete trouvé\n")
		fullDownload(client, uRL)
		return
	}
	tailleTotalFilestr := requeteInit.Header.Get("Content-Length")
	tailleTotalFile, err := strconv.Atoi(tailleTotalFilestr)
	if err != nil || tailleTotalFile == 0 {
		log.Printf("Avertissement: Taille totale du fichier inconnu")
		fullDownload(client, uRL)
		return
	}

	fileName := "file_from_url.pdf"
	file, err := os.Create(fileName)
	if err != nil {
		log.Fatalf("Impossible de creer un fichier aa a cause de l'erreur %v\n", err)
	}
	defer file.Close()
	//Boucle de telechargement par plage
	for start := 0; start < tailleTotalFile; {
		end := start + chunkSize - 1
		if end >= tailleTotalFile {
			end = tailleTotalFile - 1
		}
		rangeHeader := fmt.Sprintf("bytes=%d-%d", start, end)
		req, err := http.NewRequest("GET", uRL, nil)
		if err != nil {
			log.Fatalf("Erreur de la preparation de la requete GET: %v\n", err)
		}
		req.Header.Set("Range", rangeHeader)

		req.Header.Set("If-Range", ifRangeValue)
		reponse, err := client.Do(req)
		if err != nil {
			log.Fatalf("Erreur sur la reponse de serveur: %v\n", err)
		}

		//Verfication si a la premiere requete GET le serveur surpport Range
		if start == 0 && reponse.StatusCode == http.StatusOK {
			log.Printf("Le serveur est incompatible avec les requetes de range")
			reponse.Body.Close()
			fullDownload(client, uRL)
			return
		}
		if reponse.StatusCode != http.StatusPartialContent {
			log.Fatalf("Erreur inatendu sur les requete par plage status: %v\n", reponse.StatusCode)
		}
		bodyReponse, err := io.ReadAll(reponse.Body)
		if err != nil {
			log.Fatalf("Erreur de lecture du corps de la reponse: %v\n", err)
		}
		reponse.Body.Close()
		nbOctet, err := file.WriteAt(bodyReponse, int64(start))
		if err != nil {
			log.Fatalf("Erreur lors de la copie des donnée: %v\n", err)
		}
		start += int(nbOctet)
	}
	fmt.Printf("\nTéléchargement terminé avec succès.\n")
}

type Piece struct {
	Start        int64
	End          int64
	IfRangeValue string // Validateur pour la requête conditionnelle
}

func woker(id int, client *http.Client, url string, file *os.File, wg *sync.WaitGroup, chanels <-chan Piece) {
	defer wg.Done()

	for piece := range chanels {
		log.Printf("Worker %d : Télécharge la plage %d-%d", id, piece.Start, piece.End)
		rangeWorker := fmt.Sprintf("bytes=%d-%d", piece.Start, piece.End)
		prepareReq, err := http.NewRequest("GET", url, nil)
		if err != nil {
			log.Printf("Worker %d: Erreur de préparation de la requête: %v\n", id, err)
			continue // Passer à la tâche suivante
		}
		prepareReq.Header.Set("Range", rangeWorker)
		prepareReq.Header.Set("If-Range", piece.IfRangeValue)

		reponseServer, err := client.Do(prepareReq)
		if err != nil {
			log.Printf("Worker %d: Erreur sur la reponse du serveur: %v\n", id, err)
			continue
		}

		// Gestion du cas où le fichier a été modifié (le serveur pourrait renvoyer 200/412/etc.)
		if reponseServer.StatusCode != http.StatusPartialContent {
			log.Printf("Worker %d: Erreur de statut pour la plage %d. Statut: %v. Ce worker n'a pas pu terminer.", id, piece.Start, reponseServer.StatusCode)
			reponseServer.Body.Close()
			continue
		}
		bodyReponseServer, err := io.ReadAll(reponseServer.Body)
		if err != nil {
			log.Printf("Worker %d: Erreur sur la lecture du corps de la reponse: %v\n", id, err)
			continue
		}
		reponseServer.Body.Close()
		_, err = file.WriteAt(bodyReponseServer, piece.Start)
		if err != nil {
			log.Printf("Worker %d: Erreur d'écriture sur le fichier à l'offset %d: %v\n", id, piece.Start, err)
			continue
		}
	}
}
func downloadFileParallele(client *http.Client, url string, chunkSize int, nbWorkerFlag int) {
	var finalURL = url
	var requeteInit *http.Response
	for {
		resp, err := client.Head(finalURL)
		if err != nil {
			log.Fatalf("Erreur d'envoi de la requête HEAD vers %s: %v", finalURL, err)
		}

		// Mettre à jour requeteInit pour la portée extérieure
		requeteInit = resp

		// 1. Gérer la Redirection (3xx)
		if requeteInit.StatusCode >= 300 && requeteInit.StatusCode < 400 {
			location := requeteInit.Header.Get("Location")
			if location == "" {
				log.Fatalf("Redirection (%v) sans en-tête Location.", requeteInit.StatusCode)
			}
			log.Printf("Redirection détectée (%v). Mise à jour de l'URL vers : %s", requeteInit.StatusCode, location)

			// Fermer le corps de la réponse de redirection avant de continuer
			requeteInit.Body.Close()

			finalURL = location
			continue // Recommencer la boucle for avec la nouvelle URL
		}

		// 2. Gérer les erreurs non-200
		if requeteInit.StatusCode != http.StatusOK {
			log.Fatalf("Erreur: La requête HEAD a échoué. Statut : %v", requeteInit.StatusCode)
		}

		// Si 200 OK, la redirection est terminée.
		break
	}
	// Après le break, requeteInit contient la réponse 200 OK finale (corps non fermé).
	defer requeteInit.Body.Close()
	tailleTotalFilestr := requeteInit.Header.Get("Content-Length")
	tailleTotalFile, err := strconv.Atoi(tailleTotalFilestr)
	if err != nil || tailleTotalFile == 0 {
		log.Printf("Avertissement: Taille totale du fichier inconnu")
		fullDownload(client, finalURL)
		return
	}
	start := 0
	end := start + chunkSize - 1
	if end >= tailleTotalFile {
		end = tailleTotalFile - 1
	}
	firsPlage := fmt.Sprintf("bytes=%d-%d", start, end)
	req, err := http.NewRequest("GET", finalURL, nil)
	if err != nil {
		log.Fatalf("Erreur de la preparation de la requete GET: %v\n", err)
	}
	req.Header.Set("Range", firsPlage)
	reponse, err := client.Do(req)
	if err != nil {
		log.Fatalf("Erreur sur la reponse du serveur: %v\n", err)
	}

	if reponse.StatusCode != http.StatusPartialContent {
		log.Printf("Erreur le serveur ne support pas les requete")
		if reponse.StatusCode == http.StatusOK {
			log.Printf("Retour au telechargement complet")
			reponse.Body.Close()
			fullDownload(client, url)
			return
		}
		reponse.Body.Close()
		log.Fatalf("Erreur le serveur n'a pas repondu ni avec un status(200 ok ) ni avec (206 Partial Content) mais avec %v\n", reponse.StatusCode)
	}
	etagFirstRep := reponse.Header.Get("ETag")
	valLastModified := reponse.Header.Get("Last-Modified")
	var ifRangeValue string
	if etagFirstRep != "" {
		ifRangeValue = etagFirstRep
		log.Printf("Validateur trouvé ETag: %s\n", etagFirstRep)
	} else if valLastModified != "" {
		ifRangeValue = valLastModified
		log.Printf("Validateur trouvé Last-Modified: %s\n", valLastModified)
	} else {
		log.Printf("Aucun validateur n'a ete trouvé\n")
		reponse.Body.Close()
		fullDownload(client, finalURL)
		return
	}
	fileName := "file_from_url_parallele.pdf"
	file, err := os.Create(fileName)
	if err != nil {
		log.Fatalf("Impossible de creer un fichier aa a cause de l'erreur %v\n", err)
	}
	defer file.Close()
	var wg sync.WaitGroup
	bodyFirstPiece, err := io.ReadAll(reponse.Body)
	if err != nil {
		log.Fatalf("Erreur de lecture de la premier piece")
	}
	nbOctet, err := file.WriteAt(bodyFirstPiece, int64(start))
	start += int(nbOctet)

	//Creation d'une canal go
	var chanels chan Piece
	chanels = make(chan Piece) //canal non-buf

	for id := 0; id < nbWorkerFlag; id++ {
		wg.Add(1)
		go woker(id, client, finalURL, file, &wg, chanels)
	}
	for starts := chunkSize; starts < tailleTotalFile; starts += chunkSize {
		end := starts + chunkSize - 1
		if end >= tailleTotalFile {
			end = tailleTotalFile - 1
		}
		chanels <- Piece{Start: int64(starts), End: int64(end), IfRangeValue: ifRangeValue}

	}
	close(chanels)
	wg.Wait()
	fmt.Printf("\nTéléchargement parallèle terminé avec succès. Fichier %s, %d octets.\n", fileName, tailleTotalFile)
}
func main() {
	urlFlag := flag.String("url", "", "Telecharger un fichier")
	sizePlage := flag.Int("c", 16, "Faire des requetes sequentielles par plage(piece)\n")
	nbWorkerFlag := flag.Int("n", 4, "Le nombre de consommateur\n")
	chunkSize := *sizePlage * 1024
	flag.Parse()
	if *urlFlag == "" {
		fmt.Printf("Erreur l'option -url est obligatoire\n")
		flag.Usage()
		os.Exit(1)
	}
	client := createClient()
	//fullDownload(client, *urlFlag)
	//downloadFileSequentiel(client, *urlFlag, chunkSize)
	downloadFileParallele(client, *urlFlag, chunkSize, *nbWorkerFlag)
}
