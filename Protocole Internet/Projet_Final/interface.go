package main

import (
	"crypto/ecdsa"
	"fmt"
	"html/template"
	"log"
	"net"
	"net/http"
)

const loginPage = `
<!DOCTYPE html>
<html>
<head>
    <title>Entrez votre Pseudo</title>
    <style>
        body { font-family: sans-serif; display: flex; justify-content: center; align-items: center; height: 100vh; background: #f0f2f5; margin:0; }
        .card { background: white; padding: 2rem; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); text-align: center; }
        input { padding: 10px; width: 250px; border: 1px solid #ddd; border-radius: 4px; margin-bottom: 10px; }
        button { padding: 10px 20px; background: #007bff; color: white; border: none; border-radius: 4px; cursor: pointer; width: 100%; }
        button:hover { background: #0056b3; }
    </style>
</head>
<body>
    <div class="card">
        <h2>Initialisation Merkle P2P</h2>
        <form method="POST" action="/login">
            <input type="text" name="nickname" placeholder="Choisissez un pseudo..." required>
            <button type="submit">Se connecter et Lancer le Client</button>
        </form>
    </div>
</body>
</html>`

const dashboardPage = `
<!DOCTYPE html>
<html>
<head>
    <title>Merkle P2P Dashboard</title>
    <style>
        body { font-family: 'Segoe UI', sans-serif; margin: 0; background: #f8f9fa; }
        .nav { background: #343a40; color: white; padding: 1rem; text-align: center; display: flex; justify-content: space-around; align-items: center; }
        .container { padding: 2rem; max-width: 1000px; margin: auto; }
        table { width: 100%; border-collapse: collapse; background: white; margin-top: 1rem; box-shadow: 0 1px 3px rgba(0,0,0,0.1); }
        th, td { padding: 12px; border-bottom: 1px solid #dee2e6; text-align: left; }
        th { background: #e9ecef; }
        .btn { padding: 8px 15px; background: #28a745; color: white; border: none; border-radius: 4px; cursor: pointer; }
        .btn:hover { background: #218838; }
        .logout { color: #ffc107; text-decoration: none; font-size: 0.9rem; border: 1px solid #ffc107; padding: 5px; border-radius: 4px; }
    </style>
</head>
<body>
    <div class="nav">
        <h2>Utilisateur : {{.MyName}}</h2>
        <a href="/logout" class="logout">Changer de Pseudo</a>
    </div>
    <div class="container">
        <h3>Pairs actifs sur le réseau</h3>
        <table>
            <tr>
                <th>NickName</th>
                <th>Adresses UDP</th>
                <th>Action</th>
            </tr>
            {{range .Peers}}
            <tr>
                <td><strong>{{.Name}}</strong></td>
                <td><code>{{.Addrs}}</code></td>
                <td>
                    <form method="POST" action="/download">
                        <input type="hidden" name="target" value="{{.Name}}">
                        <button type="submit" class="btn">Télécharger</button>
                    </form>
                </td>
            </tr>
            {{end}}
        </table>
        <p style="text-align:center;"><a href="/">Rafraîchir la liste</a></p>
    </div>
</body>
</html>`

type PeerInfo struct {
	Name  string
	Addrs string
}

func StartGlobalInterface(client *http.Client) {
	mux := http.NewServeMux()
	mux.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		mapLock.RLock()
		currentName := nickName
		mapLock.RUnlock()

		if currentName == "" {
			tmpl := template.Must(template.New("login").Parse(loginPage))
			tmpl.Execute(w, nil)
		} else {
			names, _ := GetPeerList(client)
			var displayPeers []PeerInfo
			for _, name := range names {
				if name == currentName || name == "" {
					continue
				}
				addrs, _ := GetPeerAddresses(client, name)
				displayPeers = append(displayPeers, PeerInfo{
					Name:  name,
					Addrs: fmt.Sprintf("%v", addrs),
				})
			}

			tmpl := template.Must(template.New("dash").Parse(dashboardPage))
			tmpl.Execute(w, map[string]interface{}{
				"MyName": currentName,
				"Peers":  displayPeers,
			})
		}
	})
	mux.HandleFunc("/login", func(w http.ResponseWriter, r *http.Request) {
		if r.Method == http.MethodPost {
			name := r.FormValue("nickname")
			if name != "" {
				setupP2P(name, client)
				http.Redirect(w, r, "/", http.StatusSeeOther)
				return
			}
		}
		http.Redirect(w, r, "/", http.StatusSeeOther)
	})
	mux.HandleFunc("/download", func(w http.ResponseWriter, r *http.Request) {
		target := r.FormValue("target")
		if target != "" {
			go func() {
				log.Printf("Web: Démarrage téléchargement depuis %s", target)
				addrs, _ := GetPeerAddresses(client, target)
				if len(addrs) > 0 {
					DiscoveryRoutine(connGlobal, &addrs[0], privateKeyGlobal)
					relaisAddr, _ := net.ResolveUDPAddr("udp", "jch.irif.fr:8443")
					helloPack, _ := BuildHelloPacket(nickName, 1, privateKeyGlobal)
					forwardPack, _ := BuildForwardPacket(&addrs[0], helloPack)
					log.Printf("NAT: Envoi du relais via jch.irif.fr pour joindre %s", target)
					SendRequestToThePeer(connGlobal, relaisAddr, forwardPack)
				}
			}()
			fmt.Fprintf(w, "<html><body style='text-align:center;font-family:sans-serif;'><h1>Requête envoyée !</h1><p>Le téléchargement de l'arbre Merkle de <b>%s</b> a commencé.</p><a href='/'>Retour au Dashboard</a></body></html>", target)
		}
	})

	mux.HandleFunc("/logout", func(w http.ResponseWriter, r *http.Request) {
		mapLock.Lock()
		nickName = ""
		mapLock.Unlock()
		http.Redirect(w, r, "/", http.StatusSeeOther)
	})

	log.Println("Accédez à l'interface sur : http://localhost:8080")
	http.ListenAndServe(":8080", mux)
}
func setupP2P(name string, client *http.Client) {
	mapLock.Lock()
	nickName = name
	mapLock.Unlock()
	pubKey := privateKeyGlobal.Public().(*ecdsa.PublicKey)
	pubKeyBytes := FormatPubKey(pubKey)
	err := RegisterKey(client, nickName, pubKeyBytes)
	if err != nil {
		log.Printf("Erreur RegisterKey: %v", err)
	}

	serverUDPAddr, _ := net.ResolveUDPAddr("udp", "jch.irif.fr:8443")
	// On envoie un Hello avec extension 1 (NAT Traversal)
	helloPacket, err := BuildHelloPacket(nickName, 1, privateKeyGlobal)
	if err == nil {
		SendRequestToThePeer(connGlobal, serverUDPAddr, helloPacket)
		log.Printf("UDP: Enregistrement initial envoyé au serveur")
	}
}
