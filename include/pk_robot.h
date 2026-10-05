#pragma once

#include <Arduino.h>
#include <RobotEyes.h>

// ============================================================================
// PortaChiave - Modulo RobotEyes
//
// Punto unico per applicare comandi al robot, sia dalla Serial che dalle
// API del Core. La protezione (AUTH) avviene a livello API/webserver,
// qui si applica solo la logica del robot.
// ============================================================================

namespace pk_robot {

// Collega il modulo all'istanza occhi.
void begin(RobotEyes* eyes);

// Applica un comando testuale dal ponte seriale.
// Accetta: emotion:<name> anim:<name> sleep:on|off breath:on|off fps
// Ritorna true se il comando era un comando robot riconosciuto.
bool handleCommand(const String& cmd);

// Applica un'azione da API ({action, value}). Ritorna una stringa di risposta.
String apiApply(const String& action, const String& value);

// Inoltra un evento agli occhi (reazione automatica).
void reactEvent(EyeEvent event);

// Emozioni supportate pubblicamente.
String emotions();

// Riapplica al boot l'ultima emozione persistita dall'utente (se presente).
void restoreEmotion();

// Ritorna il nome dell'emozione attuale.
String currentEmotionName();

// Stato robot per /api/system
uint16_t currentFps();
bool isSleeping();

} // namespace pk_robot