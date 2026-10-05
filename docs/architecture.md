 # PortaChiave - analisi architetturale

 ## Stato rilevato

 Il progetto attuale e' un firmware PlatformIO per ESP8266 NodeMCU (`src/` e
 `include/`). Non e' presente un backend esterno ne' una web app separata:
 l'ESP8266 espone la dashboard HTML embedded in flash tramite
 `ESP8266WebServer` sulla porta 80.

 ## Flusso principale

 - `src/main.cpp` inizializza access point Wi-Fi, autenticazione, API,
	 captive portal, display SSD1306 e RobotEyes, poi esegue il loop non
	 bloccante.
 - `src/pk_api.cpp` registra gli endpoint REST e inoltra i comandi a
	 `auth`, `access`, `pk_net`, `pk_scanner` e `pk_robot`.
 - `src/pk_dashboard.cpp` contiene tutte le viste e il JavaScript client-side;
	 le chiamate browser usano `fetch` verso gli endpoint dello stesso ESP.
 - `src/wifi_ap.cpp` gestisce l'AP autonomo e lo stato dei client; non esiste
	 attualmente WebSocket, MQTT o signaling WebRTC.

 ## Sistema occhi

 `RobotEyes` disegna due forme su `Adafruit_SSD1306` tramite Adafruit GFX.
 L'hardware configurato e' monocromatico (`SSD1306_WHITE`/`SSD1306_BLACK`),
 quindi palette RGB e modalita' rosse non sono implementabili correttamente
 senza cambiare display. `pk_robot` e' il punto unico gia' usato dalle API e
 dalla seriale per applicare emozioni e animazioni.

 Le espressioni attuali sono enum/configurazioni locali a `RobotEyes`; gli
 eventi UI centralizzati, la configurazione remota degli occhi e il QR code
 non risultano ancora presenti.

 ## Implicazioni per Camera Pairing

 Il prompt descrive un'architettura browser/server con signaling WebRTC. La
 sola dashboard embedded non puo' fornire in modo robusto HTTPS pubblico,
 signaling tra dispositivi o servizio TURN: il pairing completo richiede un
 backend/web app esterno. Il firmware puo' invece ricevere lo stato e l'URL
 della sessione, mostrare il QR e riportare gli stati tramite API, mantenendo
 il video esclusivamente nel browser.

 L'implementazione deve quindi essere incrementale e compatibile: prima si
 definiscono contratti e stato firmware riutilizzando `pk_api`/`pk_robot`, poi
 si aggiunge un servizio web esterno per sessioni, QR lato server e signaling.
 Non sono state rilevate modifiche locali attribuibili a OpenCode nei file del
 progetto al momento dell'analisi; la directory non ha tuttavia una propria
 `.git` da usare come garanzia di collaborazione.
