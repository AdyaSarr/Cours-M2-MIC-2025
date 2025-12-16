/* package main

import (
	"crypto/tls"
	"fmt"
	"log"
	"net/http"


	"github.com/jech/cert"
)

func main() {
	http.HandleFunc("/page.html", page)
	//certFile := "/Users/adyasarr/Desktop/M2 MIC/Protocole Internet/TP/TP4/TP4_BIS/cert.pem"
	//keyFile := "/Users/adyasarr/Desktop/M2 MIC/Protocole Internet/TP/TP4/TP4_BIS/key.pem"
	//err := http.ListenAndServeTLS(":8443", certFile, keyFile, nil)
	//log.Fatal("ListenAndServeTLS", err)
	certificate := cert.New("certFile", "keyFile")

	//onfiguration du serveur
	s := http.Server{
		Addr: ":8443", // definir le port HTTPS
		//Configuration de la couche TLS
		TLSConfig: &tls.Config{
			GetCertificate: func(hello *tls.ClientHelloInfo) (*tls.Certificate, error) {
				return certificate.Get()
			},
		},
	}
	log.Printf("Demarrage du serveur: https://localhost:8443")
	err := s.ListenAndServeTLS("", "")
	log.Fatal("ListenAndServe", err)
}

func page(w http.ResponseWriter, r *http.Request) {
	const data = `<!DOCTYPE>
					<html>
						<head>
						</head>
						<body>
							<h1> Je teste si tout fonctionne </h1>
						</body>
					</html>`
	if r.Method != "HEAD" && r.Method != "GET" {
		http.Error(w, "Metho not allowed", http.StatusMethodNotAllowed)
		return
	}
	userName, password, present := r.BasicAuth()
	if !present || userName != "einstein" || password != "elsa" {
		w.Header().Set("WWW-Authenticate", `Basic realm="Accès restreint"`)
		http.Error(w, "Accés non autorise", http.StatusUnauthorized)
		return
	}
	log.Printf("L'utlisateur %v s'est authentifié au pres du serveur\n", userName)
	w.Header().Set("Content-Type", "text/html; charset=utf-8")
	fmt.Fprintf(w, data)
}
*/

package main

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"strings"

	"crypto/tls"
	"github.com/jech/cert"
)

// Constantes d'Authentification
const (
	VALID_PASSWORD = "Rosebud"
	SERVER_PORT    = ":8444" // Utilisation du port 8444 comme dans le TP
	SECRET_TOKEN   = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyIjoiY2F0X25hbWUiLCJpYXQiOjE2ODg1NjY2NTZ9.SGV5dGhpc2lzYXRva2Vu"
)

// Structure JSON de la Requête POST
type Credentials struct {
	Username string `json:"username"`
	Password string `json:"password"`
}

// Handler POST /get-token (Exercice 4.1)
func getTokenHandler(w http.ResponseWriter, r *http.Request) {
	if r.Method != "POST" {
		http.Error(w, "Method Not Allowed", http.StatusMethodNotAllowed)
		return
	}

	var creds Credentials

	// Lire et décoder le corps JSON
	if err := json.NewDecoder(r.Body).Decode(&creds); err != nil {
		http.Error(w, "Invalid JSON format", http.StatusBadRequest)
		return
	}

	// 1. Vérification des identifiants (selon l'énoncé du TP)
	if creds.Username == "" || creds.Password != VALID_PASSWORD {
		http.Error(w, "Invalid credentials", http.StatusUnauthorized)
		return
	}

	log.Printf("Tentative d'authentification réussie pour : %s", creds.Username)

	// 2. Créer la réponse JSON contenant le jeton
	response := map[string]string{"token": SECRET_TOKEN}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(response)
}

// Handler GET /top-secret (Exercice 4.2)
func topSecretHandler(w http.ResponseWriter, r *http.Request) {
	if r.Method != "GET" {
		http.Error(w, "Method Not Allowed", http.StatusMethodNotAllowed)
		return
	}

	// 1. Lire l'en-tête Authorization
	authHeader := r.Header.Get("Authorization")

	// Vérifier la présence du jeton
	if authHeader == "" || !strings.HasPrefix(authHeader, "Bearer ") {
		w.Header().Set("WWW-Authenticate", "Bearer realm=\"API Secret\"")
		http.Error(w, "Token manquant ou format invalide (Bearer)", http.StatusUnauthorized)
		return
	}

	// 2. Extraire et valider le jeton
	token := strings.TrimPrefix(authHeader, "Bearer ")

	if token != SECRET_TOKEN {
		http.Error(w, "Jeton invalide ou expiré", http.StatusForbidden)
		return
	}

	// 3. Succès
	w.Header().Set("Content-Type", "text/plain; charset=utf-8")
	fmt.Fprintln(w, "Accès accordé. Données secrètes : La réponse est 42.")
}

// --- MAIN SERVER SETUP (Utilisation du HTTPS comme Exercice 2) ---

func main() {
	// Enregistrement des handlers de l'Exercice 4
	http.HandleFunc("/get-token", getTokenHandler)
	http.HandleFunc("/top-secret", topSecretHandler)

	// Utilisation de github.com/jech/cert (Exercice 2.4)
	certificate := cert.New("certFile", "keyFile")

	s := http.Server{
		Addr: SERVER_PORT,
		TLSConfig: &tls.Config{
			GetCertificate: func(hello *tls.ClientHelloInfo) (*tls.Certificate, error) {
				return certificate.Get()
			},
		},
	}

	log.Printf("Serveur d'authentification local démarré sur : https://localhost%s", SERVER_PORT)
	log.Println("Utiliser n'importe quel nom d'utilisateur et le mot de passe 'Rosebud'.")

	// Démarrage en mode HTTPS
	err := s.ListenAndServeTLS("", "")
	log.Fatal("ListenAndServeTLS: ", err)
}
