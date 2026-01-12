package main

import (
	"context"
	"fmt"
	"html/template"
	"log"
	"net/http"
	"time"
)

const loginPage = `
<!DOCTYPE html>
<html>
<head>
    <title>Connexion P2P Merkle</title>
    <style>
        body { font-family: sans-serif; display: flex; justify-content: center; align-items: center; height: 100vh; background: #f0f2f5; }
        .card { background: white; padding: 2rem; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        input { padding: 10px; width: 250px; border: 1px solid #ddd; border-radius: 4px; }
        button { padding: 10px 20px; background: #007bff; color: white; border: none; border-radius: 4px; cursor: pointer; }
        .error { color: red; font-size: 0.8rem; margin-top: 5px; }
    </style>
</head>
<body>
    <div class="card">
        <h2>Entrez votre pseudo</h2>
        <form method="POST" action="/login">
            <input type="text" name="nickname" placeholder="Votre NickName..." required>
            <button type="submit">Lancer le Client</button>
            {{if .}} <p class="error">{{.}}</p> {{end}}
        </form>
    </div>
</body>
</html>`

func GetNickNameWeb() string {
	nameChan := make(chan string)

	mux := http.NewServeMux()
	server := &http.Server{Addr: ":8080", Handler: mux}
	mux.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		tmpl := template.Must(template.New("login").Parse(loginPage))
		tmpl.Execute(w, nil)
	})
	mux.HandleFunc("/login", func(w http.ResponseWriter, r *http.Request) {
		if r.Method == http.MethodPost {
			name := r.FormValue("nickname")
			if name != "" {
				fmt.Fprintf(w, "<h1>Démarrage réussi !</h1><p>Vous pouvez fermer cet onglet, le client P2P est lancé sous le nom : %s</p>", name)
				nameChan <- name
				return
			}
			tmpl := template.Must(template.New("login").Parse(loginPage))
			tmpl.Execute(w, "Le nom ne peut pas être vide")
		}
	})

	go func() {
		log.Println("Veuillez configurer votre NickName sur : http://localhost:8080")
		if err := server.ListenAndServe(); err != http.ErrServerClosed {
			log.Fatalf("Erreur serveur Web: %v", err)
		}
	}()

	chosenName := <-nameChan
	ctx, _ := context.WithTimeout(context.Background(), 5*time.Second)
	server.Shutdown(ctx)

	return chosenName
}
