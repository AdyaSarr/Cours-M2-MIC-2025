package main

//tout d'abord il faut executer cette commande pour avoir toutes les dependance necessaire: go get github.com/jech/cert
import (
	"crypto/tls"
	"fmt"
	"log"
	"net/http"
	"slices"
	"strconv"

	"github.com/jech/cert"
)

const TAILLE = 100000

func main() {
	http.HandleFunc("/form-primes", formPrimes)
	http.HandleFunc("/calculate-primes", calculatePrimes)
	//err := http.ListenAndServe(":8080", nil)//utilisation de http sans l'option de securite
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

func formPrimes(w http.ResponseWriter, r *http.Request) {
	const data = `<!DOCTYPE html>
<html>
<head></head>
<body>
<form action="/calculate-primes" method="post">
Entrez un nombre positif: <input type="number" name="number"/> <input type="submit"/></form>
</body>
</html>`
	if r.Method != "HEAD" && r.Method != "GET" {
		http.Error(w, "method not allowed", http.StatusMethodNotAllowed)
		return
	}
	w.Header().Set("Content-Type", "text/html; charset=utf-8")
	fmt.Fprintf(w, data)
}
func erathostene_algo_efficace(n int) []int {
	if n > TAILLE {
		n = TAILLE
	}
	a := make([]int, n-1)
	for i := 0; i < len(a); i++ {
		a[i] = i + 2
	}
	for i := 0; i < len(a); i++ {
		for j := i + 1; j < len(a); {
			if a[j]%a[i] == 0 {
				a = slices.Delete(a, j, j+1)
			} else {
				j++
			}
		}
	}
	return a
}
func calculatePrimes(w http.ResponseWriter, r *http.Request) {
	if r.Method != "POST" {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}
	err := r.ParseForm()
	if err != nil {
		http.Error(w, "We cannot Parse the request", http.StatusBadRequest)
		return
	}
	number_str := r.Form.Get("number")

	number, err := strconv.ParseInt(number_str, 10, 64)

	if err != nil {
		http.Error(w, "We cannot the input from a string to a integer", http.StatusBadRequest)
		return
	}
	number_final := int(number)
	data := erathostene_algo_efficace(number_final)

	w.Header().Set("Content-Type", "text/html; charset=utf-8")
	fmt.Fprintf(w, `<!DOCTYPE html>
<html>
<head></head>
<body>
<h1>La liste des nombres premier jusqu'a %d : </h1>
<p>%v</p>
</body>
</html>`, number_final, data)
}
