package main

import (
	"fmt"
	"log"
	"net/http"
)

func main() {
	http.HandleFunc("/hello.text", helloText)
	http.HandleFunc("/hello.html", helloHtml)
	http.HandleFunc("/name-get", nameGet)
	http.HandleFunc("/request-name", requestName)
	http.HandleFunc("/name-post", namePost)
	http.HandleFunc("/request-name-post", requestNamePost)
	err := http.ListenAndServe(":8080", nil)
	log.Fatal("ListenAndServe: ", err)
}

func helloText(w http.ResponseWriter, r *http.Request) {
	if r.Method != "HEAD" && r.Method != "GET" {
		http.Error(w, "method no allowed", http.StatusMethodNotAllowed)
		return
	}
	w.Header().Set("Content-Type", "text/plain; charset=utf-8")
	fmt.Fprintln(w, "Nan-nga Defff !")
}

func helloHtml(w http.ResponseWriter, r *http.Request) {
	const data = `<!DOCTYPE html>
<html>
<head></head>
<body>
<p>Nan-nga Defff !</p>
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

func requestName(w http.ResponseWriter, r *http.Request) {
	if r.Method != "HEAD" && r.Method != "GET" {
		http.Error(w, "method not allowed", http.StatusMethodNotAllowed)
		return
	}
	err := r.ParseForm()
	if err != nil {
		http.Error(w, "You could not parse the request", http.StatusBadRequest)
		return
	}
	name := r.Form.Get("name")
	w.Header().Set("Content-Type", "text/html; charset=utf-8")
	fmt.Fprintf(w, `<!DOCTYPE html>
<html>
<head></head>
<body>
<h1>You name is: %s </h1>
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

//pour afficher le contenu dsue le terminal directement
/* func requestNamePost(w http.ResponseWriter, r *http.Request) {
	if r.Method != "POST" {
		http.Error(w, "method not allowed", http.StatusMethodNotAllowed)
		return
	}

	content := r.Header.Get("Content-Type")
	log.Printf("%s", content)
	io.Copy(os.Stdout, r.Body)
} */

func requestNamePost(w http.ResponseWriter, r *http.Request) {
	if r.Method != "POST" {
		http.Error(w, "method not allowed", http.StatusMethodNotAllowed)
		return
	}
	err := r.ParseForm() //lit et vide le r.body
	if err != nil {
		http.Error(w, "We cannot parse the request", http.StatusBadRequest)
		return
	}

	name := r.Form.Get("name")
	w.Header().Set("Content-Type", "text/html; charset=utf-8")
	fmt.Fprintf(w, `<!DOCTYPE html>
<html>
<head></head>
<body>
<h1>You name is: %s </h1>
</body>
</html>`, name)
}
