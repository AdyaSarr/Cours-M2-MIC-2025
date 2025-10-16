package main

import (
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
)

func main() {
	http.HandleFunc("/hello.text", helloText)
	http.HandleFunc("/hello.html", helloHtml)
	http.HandleFunc("/name-get", nameGet)
	http.HandleFunc("/name-post", namePost)
	http.HandleFunc("/request-name", nameRequest)
	http.HandleFunc("/request-name-post", postnameRequest)
	err := http.ListenAndServe(":8080", nil)
	log.Fatal("ListenAndServe: ", err)
}

func helloText(w http.ResponseWriter, r *http.Request) {
	if r.Method != "HEAD" && r.Method != "GET" {
		http.Error(w, "method no allowed", http.StatusMethodNotAllowed)
		return
	}
	w.Header().Set("Content-Type", "text/plain; charset=utf-8")
	fmt.Fprintln(w, "Na Nga Def !")
}

func helloHtml(w http.ResponseWriter, r *http.Request) {
	const data = `<!DOCTYPE html>
<html>
<head></head>
<body>
<p>Na Nga Def !</p>
</body>
</html>`
	if r.Method != "HEAD" && r.Method != "GET" {
		http.Error(w, "method not allowed", http.StatusMethodNotAllowed)
		return
	}
	w.Header().Set("Content-Type", "text/html; charset=utf-8")
	fmt.Fprintf(w, data)
}

func nameGet(w http.ResponseWriter, r *http.Request) {
	const data = `<!DOCTYPE html>
<html>
<head></head>
<body>
<form action="/request-name" method="get">
Votre nom: <input type="text" name="name"/> <input type="submit"/></form>
</body>
</html>`
	if r.Method != "HEAD" && r.Method != "GET" {
		http.Error(w, "method not allowed", http.StatusMethodNotAllowed)
		return
	}
	w.Header().Set("Content-Type", "text/html; charset=utf-8")
	fmt.Fprintf(w, data)
}

func nameRequest(w http.ResponseWriter, r *http.Request) {
	if r.Method != "HEAD" && r.Method != "GET" {
		http.Error(w, "method not allowed", http.StatusMethodNotAllowed)
		return
	}

	err := r.ParseForm()
	if err != nil {
		http.Error(w, "We could not parse the response", http.StatusBadRequest)
		return
	}

	name := r.Form.Get("name")

	w.Header().Set("Content-Type", "text/html; charset=utf-8")

	// Correction de l'erreur : utilisation de fmt.Fprintf
	// Le verbe de formatage %s est remplacé par la valeur de 'name'
	fmt.Fprintf(w, `<!DOCTYPE html>
<html>
<head></head>
<body>
    <h1>Your name is %s</h1>
</body>
</html>`, name)
}

func namePost(w http.ResponseWriter, r *http.Request) {
	const data = `<!DOCTYPE html>
<html>
<head></head>
<body>
<form action="/request-name-post" method="post">
Votre nom: <input type="text" name="name"/> <input type="submit"/></form>
</body>
</html>`
	if r.Method != "HEAD" && r.Method != "GET" {
		http.Error(w, "method not allowed", http.StatusMethodNotAllowed)
		return
	}
	w.Header().Set("Content-Type", "text/html; charset=utf-8")
	fmt.Fprintf(w, data)
}

func postnameRequest(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "method not allowed", http.StatusMethodNotAllowed)
		return
	}

	contenttype := r.Header.Get("Content-type")
	fmt.Println("Content-type", contenttype)

	_, err := io.Copy(os.Stdout, r.Body)

	if err != nil {
		log.Println("Error copying request body:", err)
		http.Error(w, "We could not copy the body", http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "text/html; charset=utf-8")
	fmt.Fprintf(w, `<!Doctype html>
	<html>
	<head></head>
	<body>
    	<h1>Données reçues et traitées sur le serveur.</h1>
        <p>Vérifiez le terminal pour voir les détails de la requête.</p>
	</body>
	</html>`)
	io.Copy(os.Stdout, r.Body)
}
