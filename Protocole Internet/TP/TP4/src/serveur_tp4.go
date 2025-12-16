package main

import (
	"fmt"
	"log"
	"net/http"
)

func main() {
	http.HandleFunc("/primes.html", primesHTML)
	http.HandleFunc("/request-primes", requestPrimes)
	certPath := "/Users/adyasarr/cert.pem"
	keyPath := "/Users/adyasarr/key.pem"
	err := http.ListenAndServeTLS(":443", certPath, keyPath, nil)
	log.Fatal("ListenAndServe :", err)
}

func primesHTML(w http.ResponseWriter, r *http.Request) {

	const data = `<!DOCTYPE html>
					<html>
						<head></head>
						<body>
							<form action="/request-primes" method="get">
								Entrez votre user: <input type="text" name="user"/> 
								Entrez votre password <input type="password" name="pwd"/>
								<input type="submit"/>
							</form>
							</body>
					</html>`
	pass, pw, err := r.BasicAuth()
	if err != false {
		w.Header().Set("www-Authenticate:", "Basic realm \"toto\"")
		http.Error(w, "Non autoriser au client", http.StatusUnauthorized)
		return
	}
	fmt.Print(pass)
	fmt.Print(pw)

	if r.Method != "HEAD" && r.Method != "GET" {
		http.Error(w, "Method no allowed", http.StatusMethodNotAllowed)
		return
	}

	w.Header().Set("Content-Type", "text/html; charset=utfl-8")
	fmt.Fprintf(w, data)

}

func requestPrimes(w http.ResponseWriter, r *http.Request) {
	if r.Method != "HEAD" && r.Method != "GET" {
		http.Error(w, "Method no allowed", http.StatusMethodNotAllowed)
		return
	}

	err := r.ParseForm()

	if err != nil {
		http.Error(w, "We cannot parse the request", http.StatusBadRequest)
		return
	}

	pass, pw, err := r.BasicAuth()
	if err != false {
		w.Header().Set("www-Authenticate:", "Basic realm \"toto\"")
		http.Error(w, "Non autoriser au client", http.StatusUnauthorized)
		return
	}
	fmt.Print(pass)
	fmt.Print(pw)

	w.Header().Set("Content-Type", "text/html; charset=utf-8")

	fmt.Fprintf(w, `<!DOCTYPE html>
					<html>
						<head></head>
						<body>
							<h1>the primes numbers between 2 and %v sont: %v</h1>
						</body>
					</html>`, pass, pw)
}
