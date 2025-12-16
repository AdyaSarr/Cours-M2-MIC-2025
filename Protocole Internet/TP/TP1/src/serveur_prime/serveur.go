package main

import (
	"fmt"
	"log"
	"net/http"
	"slices"
	"strconv"
)

const TAILLE = 1000

func main() {
	http.HandleFunc("/primes.html", primesHTML)
	http.HandleFunc("/request-primes", requestPrimes)
	err := http.ListenAndServe(":8080", nil)
	log.Fatal("ListenAndServe :", err)
	//err := http.ListenAndServeTLS(":8443", certPath, keyPath, nil)
}

func primesHTML(w http.ResponseWriter, r *http.Request) {

	const data = `<!DOCTYPE html>
					<html>
						<head></head>
						<body>
							<form action="/request-primes" method="get">
								Entrez un entier positif: <input type="number" name="number"/> <input type="submit"/></form>
							</body>
					</html>`

	if r.Method != "HEAD" && r.Method != "GET" {
		http.Error(w, "Method no allowed", http.StatusMethodNotAllowed)
		return
	}

	w.Header().Set("Content-Type", "text/html; charset=utfl-8")
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

	numberStr := r.Form.Get("number")

	valNumber, err := strconv.ParseInt(numberStr, 10, 64)

	if err != nil {
		http.Error(w, "We cannot recover the number", http.StatusBadRequest)
		return
	}
	numberfinal := int(valNumber)

	sliceTab := erathostene_algo_efficace(numberfinal)

	w.Header().Set("Content-Type", "text/html; charset=utf-8")
	fmt.Fprintf(w, `<!DOCTYPE html>
					<html>
						<head></head>
						<body>
							<h1>the primes numbers between 2 and %d sont: %v</h1>
						</body>
					</html>`, numberfinal, sliceTab)
}
