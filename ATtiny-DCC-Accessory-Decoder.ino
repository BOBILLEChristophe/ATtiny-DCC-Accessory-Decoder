/*
 * ============================================================================================
 *  Projet : Commande de feu mécanique ou d'aiguille par servomoteur
 *  Vers : 0.6
 * --------------------------------------------------------------------------------------------
 *  Compatible : ATtiny85 + Servo_ATTinyCore + DCC_Decoder
 *  Description :
 *    - Reçoit les ordres DCC d'une centrale via la bibliothèque DCC_Decoder
 *    - Actionne un servomoteur représentant un signal mécanique
 *    - Synchronise les LEDs rouge et verte selon la position du signal
 *    - Utilise une machine à états
 * ============================================================================================
 */

#ifndef __AVR_ATtiny85__
#error "Sélectionnez la carte : ATtiny85 dans l'IDE Arduino"
#else

#include <Servo_ATTinyCore.h>  // Bibliothèque servo spécifique à l'ATtinyCore
#include <DCC_Decoder.h>       // Bibliothèque de décodage des trames DCC

#define kDCC_INTERRUPT 0  // Numéro d'interruption utilisé pour le signal DCC (pin PB2 par défaut)

// =============================================================================================
//                         REGLAGES SPECIFIQUES A VOTRE CONFIGURATION
// =============================================================================================

// Adresse DCC attribuée à ce décodeur (ex: 150)
constexpr uint16_t DCC_ADDRESS = 150;
// Position du servo (en microsecondes) pour les deux états
constexpr uint16_t ANGLE_OPEN = 800;     // Carré ouvert (voie libre)
constexpr uint16_t ANGLE_CLOSED = 1800;  // Carré fermé (arrêt)


// Vitesse de déplacement du servo (plus petit = plus rapide)
constexpr uint8_t SERVO_SPEED = 5;  // Délai (en millisecondes) entre deux pas de mouvement

// =============================================================================================
//                            PARAMÈTRES GÉNÉRAUX ET MATÉRIELS
// =============================================================================================

// Brochage sur l'ATtiny85
constexpr uint8_t SERVO_PIN = PB0;      // Signal de commande du servomoteur
constexpr uint8_t RED_LED = PB3;        // LED rouge : signal d'arrêt
constexpr uint8_t GREEN_LED = PB4;      // LED verte : voie libre
constexpr uint8_t INTERRUPT_PIN = PB2;  // Entrée pour le signal DCC


// =============================================================================================
//                           DÉFINITION DE LA MACHINE D'ÉTATS
// =============================================================================================

/*
 * La machine d'états contrôle le déroulement du signal :
 *   WAIT_COMMAND   : attente d'une trame DCC pour ce décodeur
 *   APPLY_COMMAND  : application d'un ordre reçu
 *   MOVING         : mouvement progressif du servo
 *   HOLD           : fin de mouvement, le servo est désactivé
 */

enum State {
  WAIT_COMMAND,
  APPLY_COMMAND,
  MOVING,
  HOLD
};

//State state = WAIT_COMMAND;  // État initial
volatile State state = WAIT_COMMAND; // État initial

// =============================================================================================
//                           VARIABLES GLOBALES DE FONCTIONNEMENT
// =============================================================================================

// Objet servo
Servo servo;

// Positions du servo
uint16_t currentPos = ANGLE_OPEN;  // Position actuelle
uint16_t targetPos = ANGLE_OPEN;   // Position cible

// Variables de commande
volatile bool output = false;    // État du signal (0=voie libre, 1=arrêt)
uint32_t lastMove = 0;  // Chrono interne pour temporiser les pas du servo

// =============================================================================================
//                        GESTION DU DÉCODAGE DCC ET DES COMMANDES
// =============================================================================================

/*
 * Cette fonction est appelée automatiquement à chaque réception d'une trame DCC "accessoire".
 * Elle vérifie si l'adresse reçue correspond à celle du module, puis met à jour la commande.
 */
void BasicAccDecoderPacket_Handler(int address, boolean activate, byte data) {
  // Conversion de l'adresse NMRA en adresse "décodeur accessoire étendue"
  address = (address - 1) * 4 + 1 + ((data & 0x06) >> 1);

  // Si l'adresse correspond à celle de ce décodeur :
  if (address == DCC_ADDRESS) {
    // Lecture du bit d'activation : 1 = ON (carré fermé), 0 = OFF (voie libre)
    output = (data & 0x01) != 0;
    // Déclenche la transition vers l'application de la commande
    state = APPLY_COMMAND;
  }
}

// =============================================================================================
//                                         SETUP
// =============================================================================================

void setup() {

  // Configuration des LED
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  // État initial : voie libre
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);

  // Configuration du décodeur DCC
  DCC.SetBasicAccessoryDecoderPacketHandler(BasicAccDecoderPacket_Handler, true);
  DCC.SetupDecoder(0x00, 0x00, kDCC_INTERRUPT);

}

// =============================================================================================
//                                         LOOP
// =============================================================================================




void loop() {

  //Boucle principale du décodeur DCC
  DCC.loop();

  // Machine d'états
  switch (state) {

    // État d'attente : rien à faire tant qu'aucune commande n'est reçue
    case WAIT_COMMAND:
      break;

    // Application d'une commande DCC : mise à jour de la cible et des LEDs
    case APPLY_COMMAND:
      // Position cible selon l'ordre reçu
      targetPos = output ? ANGLE_CLOSED : ANGLE_OPEN;

      // LEDs synchronisées avec l'ordre
      digitalWrite(RED_LED, output);     // Rouge = actif (arrêt)
      digitalWrite(GREEN_LED, !output);  // Vert = inactif (voie libre)

      // Activation du servo
      servo.attach(SERVO_PIN);

      // Transition vers le mouvement
      state = MOVING;
      lastMove = millis();
      break;

    // Mouvement progressif du servo
    case MOVING:
      // Temporisation non bloquante pour lisser le mouvement
      if (millis() - lastMove >= SERVO_SPEED) {
        lastMove = millis();

        // Incrémentation ou décrémentation selon la direction
        if (currentPos < targetPos) ++currentPos;
        if (currentPos > targetPos) --currentPos;

        // Commande du servo
        servo.writeMicroseconds(currentPos);

        // Si la position cible est atteinte → fin du mouvement
        if (currentPos == targetPos) state = HOLD;
      }
      break;

    // Fin de mouvement : désactivation du servo pour éviter qu’il force
    case HOLD:
      servo.detach();        // Libère le PWM pour réduire la consommation
      state = WAIT_COMMAND;  // Retour à l’attente d’un nouvel ordre
      break;
  }
}

// =============================================================================================
//                                   FIN DU PROGRAMME
// =============================================================================================

#endif
