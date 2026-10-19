# App Mobile PortaChiave

Applicazione Flutter multipiattaforma per iOS e Android per il controllo del dispositivo PortaChiave.

## Prerequisiti

- Flutter SDK versione più recente
- Android Studio o VS Code
- Dispositivo Android o iOS
- Rete locale PortaChiave-XXXX

## Installazione

1. Aprire il progetto in VS Code o Android Studio
2. Eseguire `flutter pub get` per installare le dipendenze
3. Collegare il dispositivo o avviare l'emulatore
4. Assicurarsi di essere nella rete locale PortaChiave-XXXX

## Apertura in VS Code e Android Studio

- VS Code: aprire la cartella `mobile_app/` e premere F5 o usare il pannello Flutter
- Android Studio: aprire `mobile_app/` e sincronizzare i file Gradle

## Esecuzione dell'app

1. Eseguire `flutter run` per avviare l'app sul dispositivo
2. Oppure costruire l'APK con `flutter build apk --debug`

## Connessione alla rete PortaChiave

1. Collegare il telefono alla rete Wi-Fi `PortaChiave-XXXX`
2. Nota: la rete potrebbe non avere accesso a Internet
3. L'app rileverà il dispositivo automaticamente
4. Premere il pulsante "Connetti a PortaChiave" nella schermata di onboarding
5. Oppure andare alla schermata "Connessione" e inserire `http://192.168.4.1`

## Endpoint reali utilizzati

L'app comunica con il firmware PortaChiave tramite questi endpoint HTTP locali:

- `GET /api/device` - Informazioni dispositivo (nome, ID, firmware, memoria, uptime)
- `GET /api/system` - Stato sistema (uptime, heap gratuito, motivo reset)
- `GET /api/wifi/scan` - Scanning reti Wi-Fi
- `GET /api/auth/status` - Stato autenticazione
- `POST /api/auth/login` - Accesso con credenziali
- `POST /api/auth/logout` - Uscita
- `POST /api/robot` - Controllo occhi robotici (con parametro `action`)
- `GET /api/device/camera/status` - Stato fotocamera

## Limiti del firmware

- Alcune funzionalità non sono disponibili perché non supportate dal firmware:
  - Comandi emotion/sleep/animation: usati tramite `POST /api/robot` con `action`, non endpoint separati
  - Scanner/network attack: non disponibili nell'app per motivi di sicurezza
  - Evil twin/wardriving: disabilitati nell'app
- Il firmware restituisce JSON diretto senza envelope `success/data`
- I nomi dei campi devono corrispondere a quelli del firmware (`freeHeap` non `freeMemory`)

## Limiti di sicurezza noti

- L'app non include funzioni di attacco Wi-Fi, sniffing, deauth, evil twin, wardriving
- Nessun servizio cloud, analytics, pubblicità o WebView esterno
- L'app funziona solo nella rete locale del dispositivo
- Non vengono salvate password o token in chiaro
- L'accesso automatico alla rete Wi-Fi non è supportato

## Funzioni volontariamente escluse

- Endpoint relativi a attacchi Wi-Fi (deauth, sniffing)
- Endpoint evil twin/wardriving
- Comandi non supportati dal firmware (emotion/animation separati)
- Scanner indipendente
- Rilevamento/raccolta credenziali