#include "battleship.h"

// Ship info
// Define the ship sizes here (quantity of values must correspond with the NUM_SHIPS in battleship.h file):
const int SHIP_SIZES[NUM_SHIPS] = {2, 3, 3, 4, 5};

// Display texts - Change texts according to your language:
const char *patrol_name = "Patrouillen-Boot";
const char *destroyer_name = "Zerstoerer";
const char *uboat_name = "U-Boot";
const char *cruiser_name = "Schlachtschiff";
const char *aircraft_carrier_name = "Flugzeugtraeger";
const char *ship_destroyed = " versenkt";
String ship_hit = "Schiff getroffen";
String ship_missed = "verfehlt";
String game_won = "Spiel gewonnen";
String game_lost = "Spiel verloren";
const char *waiting_player2 = "Warten auf Spieler 2";
char waiting_player1[32] = "Warten auf Spieler 1";
char welcome_player2[32] = "Willkommen Spieler 2";
String player2_ship_status;
int players_round = 0; // players turn - which player is shooting next, start with player 0 (= Spieler an der Reihe)

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 5000; // 5 seconds

char show_text[] = "show_text";
char ship_setup[] = "ship_setup";
char coordinates_msg[] = "coordinates";
char PreviewLED[] = "PreviewLED";
char HitLED[] = "HitLED";
char MissedLED[] = "MissedLED";
char ship_status[] = "ship_status";
char empty[] ="";


Player players[2]; // Array to store players

void initializeGame();
void setupShip(int playerIndex, int shipIndex);
String getShipName(int shipIndex);
void processShootCommand(int playerIndex, int opponentIndex, int shotX, int shotY);
void processShootCommand(int playerIndex, int opponentIndex, int shotX, int shotY);
void getUserInputForShootCommand(int &shotX, int &shotY, int playerIndex);
bool isGameOver(int playerIndex);
bool isGameOver(int playerIndex);
void receiveShootCommand();
void printBattlefield(int playerIndex);
void displayHits(int playerIndex);




void initializeGame()
{
  // Initialize players
  for (int i = 0; i < 2; ++i)
  {
    for (int j = 0; j < NUM_SHIPS; ++j)
    {
      players[i].ships[j].size = SHIP_SIZES[j];
      players[i].ships[j].hits = 0;
    }

    // Clear the player's battlefield
    for (int row = 0; row < BOARD_SIZE; ++row)
    {
      for (int col = 0; col < BOARD_SIZE; ++col)
      {
        players[i].battlefield[row][col] = ' ';
      }
    }

    // Set up ships for each player
    for (int j = 0; j < NUM_SHIPS; ++j)
    {
      setupShip(i, j);
    }
    // tell player 2 to wait - as player 1 will be first to set up his ships
    delay(500); 
    //sendMessage("show_text", waiting_player1);

  }
}

void setupShip(int playerIndex, int shipIndex)
{
  Ship &ship = players[playerIndex].ships[shipIndex];

  Serial.print("Player ");
  Serial.print(playerIndex + 1);
  Serial.print(", set up Ship of size ");
  Serial.println(ship.size);
  Serial.println(getShipName(shipIndex));
  Serial.println(": ");

  // Get orientation from user
  Serial.print("Enter orientation ('H' for horizontal, 'V' for vertical): ");

  if (playerIndex == 0)
  {

    // tell player 2 to wait
    sendMessage(show_text, waiting_player1);

    // load orientation screen
    loadScreen3(getShipName(shipIndex));
    // wait for user input
    while (true)
    {
      if (orientation == 'H' || orientation == 'V')
      {
        // set ship orientation after input received
        ship.orientation = orientation;
        break;
      }

      lv_task_handler(); /* let the GUI do its work */
      lv_tick_inc(5);    /* tell LVGL how much time has passed */
      delay(5);          /* let this time pass */
    }

    // clearScreen & orientation
    orientation = 'X';
    clearScreen();

    // load coordinate screen
    loadScreen1(getShipName(shipIndex));
    // screen_loop();

    // get the user input for the coordinates
    while (true)
    {

      lv_task_handler(); /* let the GUI do its work */
      lv_tick_inc(5);    /* tell LVGL how much time has passed */
      delay(5);          /* let this time pass */

      // Length of coordinates not longer then 2; first character must be alpha; second character must be numeric; (if third character is 0 then 2nd char must be 1 (because only 0 is allowed e.g. A10)) or length =2, so e.g. B7
      if (coordinates.length() >= 2 && isalpha(coordinates[0]) && isdigit(coordinates[1]) && ((coordinates[2] == '0' && coordinates[1] == '1') || coordinates.length() == 2))
      {
        // Validate that the ship does not exceed the board depending on orientation
        bool validPlacement = false;
        bool overlap = false;
        // help variables
        int startX = coordinates[0] - 'A';
        int ship_startY = 0;
        // calculate ship_startY for several validations
        for (size_t i = 1; i < coordinates.length(); ++i)
        // this handles in case of Y coordinate is 10
        {
          if (isdigit(coordinates[i]))
          {
            ship_startY = ship_startY * 10 + (coordinates[i] - '0');
          }
        }
        ship_startY -= 1; // Adjust for 0-based indexing

        // space in the code for visual separation
        // space
        // space
        // space
        // space
        // space

        if (ship.orientation == 'H')
        {
          // check ship out of bounds for horizontally for the borad
          if ((coordinates[0] - 'A' + ship.size) <= BOARD_SIZE)
          {
            // check if ships are placed on top of each other
            for (int k = 0; k < ship.size; ++k)
            {
              if (players[playerIndex].battlefield[startX + k][ship_startY] == 'S')
              {
                overlap = true;
                break;
              }
            }
            if (overlap == false)
            {
              validPlacement = true;
            }
          }
        }
        else if (ship.orientation == 'V')
        {
           // check ship out of bounds for horizontally for the borad
          if ((ship_startY + ship.size) <= BOARD_SIZE)
          {
            // check if ships are placed on top of each other
            for (int k = 0; k < ship.size; ++k)
            {
              if (players[playerIndex].battlefield[startX][ship_startY + k] == 'S')
              {
                overlap = true;
                break;
              }
            }

            if (overlap == false)
            {
              validPlacement = true;
            }


          }
        }

        // Serial.println ("Placement valid: " + String(validPlacement));

        if (validPlacement)
        {

          ship.startX = coordinates[0] - 'A';
          ship.startY = 0;
          for (size_t i = 1; i < coordinates.length(); ++i)
          {
            if (isdigit(coordinates[i]))
            {

              ship.startY = ship.startY * 10 + (coordinates[i] - '0');
            }
            else
            {

              // Handle non-digit characters in the numeric part
              Serial.println("Invalid input. Please enter in the format 'A1' to 'J10'.");
              return;
            }
          }
          ship.startY -= 1; // Adjust for 0-based indexing

          break;
        }
      }
    }
    // clearScreen
    clearScreen();
  }
  else
  {
    // player 2  - send display to player 2 and wait for data from player 2
    loadScreen2(waiting_player2);
    char shipName[32];
    getShipName(shipIndex).toCharArray(shipName, sizeof(shipName));
    sendMessage(ship_setup, shipName);

    while (message_received == false)
    { // wait until message received - this variable is changed in communications.cpp-> OnDataRecv() once user has put in the coordinates
      delay(100);
    }

    Serial.println("<<<<<<");
    Serial.println(orientation_opponent);
    Serial.println(coordX);
    Serial.println(coordY);
    Serial.println("<<<<<<");

    // data is already coming correct from client
    ship.orientation = orientation_opponent[0];
    ship.startX = coordX;
    ship.startY = coordY;

    message_received = false; // reset status - to wait for next message
  }

  delay(10);
  Serial.print("Ship coordinate 1 : ");
  Serial.println(ship.startX);
  Serial.print("Ship coordinate 2 : ");
  Serial.println(ship.startY);
  // clear screen & coordinates
  clearScreen();
  coordinates = "";

  // Place ship on the player's battlefield
  for (int i = 0; i < ship.size; ++i)
  {
    if (ship.orientation == 'H')
    {
      players[playerIndex].battlefield[ship.startX + i][ship.startY] = 'S';
      Serial.println("Ship placed Horizontal");
    }
    else
    {
      players[playerIndex].battlefield[ship.startX][ship.startY + i] = 'S';
      Serial.println("Ship placed Vertical");
    }
  }
}

String getShipName(int shipIndex)
{
  switch (shipIndex)
  {
  case 0:
    return patrol_name;
    break;
  case 1:
    return destroyer_name;
    break;
  case 2:
    return uboat_name;
    break;
  case 3:
    return cruiser_name;
    break;
  case 4:
    return aircraft_carrier_name;
    break;
  default:
    return "Invalid Ship index. Please enter a number between 0 and 4.";
    break;
  }
}

void processShootCommand(int playerIndex, int opponentIndex, int shotX, int shotY)
{

  // shot preview
  if (playerIndex == 0)
  {
    // if player 1 show on his LED matrix
    lightUpPreviewLED(shotX, shotY);
  }
  else
  {

    // send command to show on player 2 LED matix
    int coordX = shotX;
    int coordY = shotY;

    sendMessage(PreviewLED, empty, coordX, coordY);
    // Wait 3 sec for the other display to show the animation
    delay(3200);
  }

  // Check if the shot hits a ship
  if (players[opponentIndex].battlefield[shotX][shotY] == 'S')
  {
    // Ship is hit
    // Serial.println("Hit!");

    // Mark the hit on the opponent's battlefield
    players[opponentIndex].battlefield[shotX][shotY] = 'X';

    // Find the ship that was hit and update its hits
    for (int i = 0; i < NUM_SHIPS; ++i)
    {
      Ship &ship = players[opponentIndex].ships[i];

      // check horizontal ships
      if (ship.orientation == 'H' && shotX >= ship.startX && shotX < ship.startX + ship.size && shotY == ship.startY)
      {
        ship.hits++;

        if (playerIndex == 0)
        {
          // if player 1 show on his LED matrix
          lightUpHitLED(shotX, shotY);
        }
        else
        {
          // send command to show on player 2 LED matix

          int coordX = shotX;
          int coordY = shotY;

          sendMessage(HitLED, empty, coordX, coordY);
          delay(100);
        }

        Serial.println("Schiff getroffen <<<<<<<< Sende zu player2");
        // No need to differenciate between player 1 and 2. Show on both screens that ship was hit.
        // tell player 2
        String ship_hit_text = ship_hit + " " + String(char(shotX + 65)) + String(shotY + 1); // convert the int coordinates back to aphlanumeric
        char ship_hit_player2[32];
        strcpy(ship_hit_player2, ship_hit_text.c_str());
        sendMessage(show_text, ship_hit_player2);

        // show on player 1 screen
        Serial.println("Zeige auf player 1 screen <<<<<<<<<");
        loadScreen2(ship_hit_text.c_str());
        clearScreen();

        // Check if the ship is destroyed
        if (ship.hits == ship.size)
        {
          Serial.println("Ship destroyed!");
          Serial.println(getShipName(i));
          String ship_name_destroyed = getShipName(i) + ship_destroyed;

          // tell player 2 as well. Some conversion of the text necessary
          char ship_destroyed_player2[32];
          strcpy(ship_destroyed_player2, ship_name_destroyed.c_str());
          sendMessage(show_text, ship_destroyed_player2);

          // show on player 1 screen
          loadScreen2(ship_name_destroyed.c_str()); // convert String to Char*
          // No need to differenciate between player 1 and 2. Show on both screens that ship was hit.
          clearScreen();
        }
        break;
      } // check vertical ships
      else if (ship.orientation == 'V' && shotX == ship.startX && shotY >= ship.startY && shotY < ship.startY + ship.size)
      {
        ship.hits++;
        Serial.println("Ship hit: ");
        Serial.println(getShipName(i));

        if (playerIndex == 0)
        {
          // if player 1 show on his LED matrix
          lightUpHitLED(shotX, shotY);
        }
        else
        {
          // send command to show on player 2 LED matix
          int coordX = shotX;
          int coordY = shotY;

          sendMessage(HitLED, empty, coordX, coordY);
          delay(100);
        }

        // No need to differenciate between player 1 and 2. Show on both screens that ship was hit.
        // tell player 2
        Serial.println("Schiff getroffen <<<<<<<< Sende zu player2");
        String ship_hit_text = ship_hit + " " + String(char(shotX + 65)) + String(shotY + 1); // convert the int coordinates back to aphlanumeric
        char ship_hit_player2[32];
        strcpy(ship_hit_player2, ship_hit_text.c_str());
        sendMessage(show_text, ship_hit_player2);

        Serial.println("Zeige auf player 1 screen <<<<<<<<<");
        // show on screen for player1
        loadScreen2(ship_hit_text.c_str());
        // loadScreen2(ship_hit);
        clearScreen();

        // Check if the ship is destroyed
        if (ship.hits == ship.size)
        {
          Serial.println("Ship destroyed!");
          String ship_name_destroyed = getShipName(i) + ship_destroyed;
          // tell player 2 as well. Some conversion of the text necessary
          char ship_destroyed_player2[32];
          strcpy(ship_destroyed_player2, ship_name_destroyed.c_str());
          sendMessage(show_text, ship_destroyed_player2);

          // show on player 1 screen
          loadScreen2(ship_name_destroyed.c_str()); // convert String to Char*
          clearScreen();
        }
        break;
      }
    }
  }
  else
  {
    // Missed the ship
    Serial.println("Miss!");

    if (playerIndex == 0)
    {
      // if player 1 show on his LED matrix
      lightUpMissedLED(shotX, shotY);
    }
    else
    {
      // send command to show on player 2 LED matix
      /*char coordinates_opponent[16];
      coordinates_opponent[0] = shotX;
      if (shotY == 10)
      { // X = 10 --> should be handled differently but is the easiest way
        coordinates_opponent[1] = 'X';
      }
      else
      {
        coordinates_opponent[1] = shotY;
      }*/

      int coordX = shotX;
      int coordY = shotY;

      sendMessage(MissedLED, empty, coordX, coordY);
    }

    // tell player 2
    String missed_text = ship_missed + " " + String(char(shotX + 65)) + String(shotY + 1); // convert the int coordinates back to aphlanumeric
    char missed_player2[16];
    strcpy(missed_player2, missed_text.c_str());
    sendMessage(show_text, missed_player2);
    // sendMessage("show_text", ship_missed_player2);

    // show on player 1 screen
    loadScreen2(missed_text.c_str());
    clearScreen();

    // neu
    if (use_rockets == 1 || use_rockets == 2) {
      Serial.println("Rocket used - shoot again");
   
    }
    else
    {
      players_round = 1 - players_round;
    }
    // switch to next player
   
  }
}

void getUserInputForShootCommand(int &shotX, int &shotY, int playerIndex)
{
  Serial.print("Player ");
  Serial.print(playerIndex + 1);
  Serial.print(": Enter your shot (e.g., B3): ");

  // player 1 round
  if (playerIndex == 0)
  {
    // tell player 2 to wait & show the ship status
    displayHits(1); // fill the String which contains the status of the ships for player2
    delay(100);
    char player2_ship_status_char[32];
    player2_ship_status.toCharArray(player2_ship_status_char, sizeof(player2_ship_status_char)); // convert String to Char[32]
    Serial.print("Array Content:");
    Serial.println(player2_ship_status);

     delay(500); // wait 2 seconds before sending the message to ensure that player 1 has enough time display the ship status
    sendMessage(show_text, waiting_player1);
    delay(1000); // wait 2 seconds before sending the message to ensure that player 1 has enough time display the ship status

    sendMessage(ship_status, player2_ship_status_char);
    delay(100);



    player2_ship_status = ""; // clean the ship status text for Player2

    //neu
    if (players[playerIndex].rockets_ammo > 0) {
         // choose weapon
      loadScreenChooseWeapon(players[playerIndex].rockets_ammo); // neu

    // neu
      // wait for user input for weapon choice
     while (true)
    {
      if ( use_rockets == 1 || use_rockets == 2) {
        Serial.println("Player has chosen weapon");
        // reduce ammmo by 1
        players[playerIndex].rockets_ammo--;
        break;
      } else if (use_rockets == 0) {
        Serial.println("Player has chosen normal shot");
        break;
      }
      
      lv_task_handler();
      lv_tick_inc(5);
      delay(5);
    }
    clearScreen();

    }






    // load coordinate screen
    loadScreen1();
    // screen_loop();

    // get the user input for the coordinates
    while (true)
    {

      lv_task_handler();
      lv_tick_inc(5);
      delay(5);

      // Length of coordinates not longer then 2; first character must be alpha; second character must be numeric; (if third character is 0 then 2nd char must be 1 (because only 0 is allowed e.g. A10)) or length =2, so e.g. B7
      if (coordinates.length() >= 2 && isalpha(coordinates[0]) && isdigit(coordinates[1]) && ((coordinates[2] == '0' && coordinates[1] == '1') || coordinates.length() == 2))
      {
        // if (coordinates.length() >= 2 && isalpha(coordinates[0]) && isdigit(coordinates[1]) && coordinates[2] < 1) {

        shotX = coordinates[0] - 'A';
        shotY = 0;
        for (size_t i = 1; i < coordinates.length(); ++i)
        {
          if (isdigit(coordinates[i]))
          {
            shotY = shotY * 10 + (coordinates[i] - '0');
          }
          else
          {
            // Handle non-digit characters in the numeric part
            Serial.println("Invalid input. Please enter in the format 'A1' to 'J10'.");
            return;
          }
        }
        shotY -= 1; // Adjust for 0-based indexing
        break;
      }
    }
  }
  else
  {
    // player 2  - send display to player 2 and wait for data from player 2
    // clearScreen();
    displayHits(0);
    loadScreen2(waiting_player2);
    // clearScreen();

    sendMessage(coordinates_msg);

    while (message_received == false)
    { // wait until message received
      delay(100);
    }

    // new logic
    Serial.print(coordX);
    Serial.print(coordY);
    Serial.println("<<<<<<");
    shotX = coordX;
    shotY = coordY;

    // neu - get the weapon choice from player 2 - it is saved in the orientation_opponent variable because it is not needed anymore after setup ship
    Serial.println("Weapon choice received from player 2: ");
    Serial.println(orientation_opponent);
    use_rockets = atoi(orientation_opponent);

    message_received = false; // status zurücksetzen, um auf die nächste Nachricht warten zu können
  }

  delay(10);
  Serial.print("Shoot coordinate 1 : ");
  Serial.println(shotX);
  Serial.print("Shoot coordinate 2 : ");
  Serial.println(shotY);
  clearScreen();
  coordinates = "";
}

void sendGameState()
{
  // Placeholder for sending the game state using ESP Now
  // (You need to implement this based on ESP Now communication)
}

bool isGameOver(int playerIndex)
{
  // Check if the game is over by checking if all ships of one player are destroyed
  // for (int i = 0; i < 2; ++i) {
  for (int j = 0; j < NUM_SHIPS; ++j)
  {
    if (players[playerIndex].ships[j].hits < players[playerIndex].ships[j].size)
    {
      // Serial.print("player: ");
      // Serial.println(playerIndex);
      // Serial.println();
      // Serial.print("ship hits: ");
      // Serial.println(players[playerIndex].ships[j].hits);
      // delay(100);
      // Serial.print("Size: ");
      // Serial.println(players[playerIndex].ships[j].size);
      // delay(100);
      return false;
    }
  }
  //}
  return true;
}

void handleGameOver(int players_round)
{

  if (players_round == 0)
  {
    // send loose screen to other player
    char game_lost_player2[32];
    strcpy(game_lost_player2, game_lost.c_str());
    sendMessage(show_text, game_lost_player2);

    // show player 1 he has won
    loadScreen2(game_won.c_str());
    clearScreen();
  }
  else
  {
    // send win screen to other player
    char game_won_player2[32];
    strcpy(game_won_player2, game_won.c_str());
    sendMessage(show_text, game_won_player2);

    // show player 1 he has lost
    loadScreen2(game_lost.c_str());
    clearScreen();
  }

  Serial.println("---------------game over---------------");
  delay(10000);
}

// only for testing porposes
void printBattlefield(int playerIndex)
{
  // Print player's own battlefield
  Serial.println("Your Ships:");
  Serial.print("   ");
  for (int i = 0; i < BOARD_SIZE; ++i)
  {
    Serial.print((char)('A' + i));
    Serial.print(" ");
  }
  Serial.println();
  for (int row = 0; row < BOARD_SIZE; ++row)
  {
    Serial.print(row + 1);
    Serial.print(" |");
    for (int col = 0; col < BOARD_SIZE; ++col)
    {
      // Show ships on the player's battlefield
      if (players[playerIndex].battlefield[col][row] == 'S')
      {
        Serial.print("S ");
      }
      else
      {
        Serial.print(players[playerIndex].battlefield[col][row]);
        Serial.print(" ");
      }
    }
    Serial.println();
  }

  Serial.println("other Ships:");
  Serial.print("   ");
  for (int i = 0; i < BOARD_SIZE; ++i)
  {
    Serial.print((char)('A' + i));
    Serial.print(" ");
  }
  Serial.println();
  for (int row = 0; row < BOARD_SIZE; ++row)
  {
    Serial.print(row + 1);
    Serial.print(" |");
    for (int col = 0; col < BOARD_SIZE; ++col)
    {
      // Show ships on the player's battlefield
      if (players[1 - playerIndex].battlefield[col][row] == 'S')
      {
        Serial.print("S ");
      }
      else
      {
        Serial.print(players[1 - playerIndex].battlefield[col][row]);
        Serial.print(" ");
      }
    }
    Serial.println();
  }

  Serial.println();
}

void displayHits(int playerIndex)
{
  Serial.println("Hits for each ship:");
  int abstandX = -40;
  int abstandY = -60;
  String status;
  for (int i = 0; i < NUM_SHIPS; ++i)
  {
    // Iterate through each field of the ship

    if (i == 2 || i == 4)
    {
      // start new line
      abstandY += 20;
      abstandX = -50; // reset X
    }
    for (int j = 0; j < players[playerIndex].ships[i].size; ++j)
    {
      // Print 'X' for hits and 'O' for non-hit fields
      status = status + (players[playerIndex].ships[i].hits > j ? 'X' : 'O');
    }

    if (playerIndex == 0)
    {                       // player1
      Serial.print(status); // print ship status to console
      Serial.println();
      shipStatus(status.c_str(), abstandX, abstandY);
      abstandX += 60;
      status = "";
    }
    else
    {                                                           // player 2
      player2_ship_status = player2_ship_status + status + ","; // combine all ships status to one string seperated by ","
      status = "";
    }
  }
  Serial.print(player2_ship_status);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting Setup");

  // inizialize LED and make simple test
  ledInitalize();

  // connect to other player
  initalizeCommunication();
  // send message and wait until it was received, otherwise send again - I could do this every time  I send a message if message sending is unreliable
  while (true)
  {
    unsigned long currentTime = millis();
    // send again after 5 seconds
    if (currentTime - lastSendTime >= sendInterval)
    {
      sendMessage(show_text, welcome_player2);
      lastSendTime = currentTime;
      Serial.println("Gesendet");
    }
    else if (send_status_received == true)
    {
      Serial.println("erhalten");
      send_status_received = false;
      break;
    }
    delay(100);
  }

  // Register for a callback function that will be called when data is received
  registerCallback();

  // Initialize Display / LVGL
  initializeDisplay();
  Serial.println("Display ok");

  initializeGame();
}

void loop()
{
  // Main game loop

  while (!isGameOver(0))
  {

    int shotX, shotY;
    
    
    getUserInputForShootCommand(shotX, shotY, players_round);

    //neu
    if (use_rockets == 1) {
      Serial.println("Player has chosen rockets horizontal");
      // process shoot command for rockets
      processShootCommand(players_round, (1 - players_round), shotX, shotY);
      processShootCommand(players_round, (1 - players_round), shotX + 1, shotY); // shoot right
      use_rockets = 3; // last shot - reset rockets status for next round
      processShootCommand(players_round, (1 - players_round), shotX + 2, shotY); // shoot left
    } else if (use_rockets == 2) {
      Serial.println("Player has chosen rockets vertical");
      // process shoot command for rockets
      processShootCommand(players_round, (1 - players_round), shotX, shotY); // shoot middle
      processShootCommand(players_round, (1 - players_round), shotX, shotY + 1); // shoot down
      use_rockets = 3; // last shot - reset rockets status for next round
      processShootCommand(players_round, (1 - players_round), shotX, shotY + 2); // shoot up
    }
    
    else {
      Serial.println("Player has chosen cannon");

    processShootCommand(players_round, (1 - players_round), shotX, shotY); // Assuming player 1 is index 0, and player 2 is index 1
    printBattlefield(players_round);
    use_rockets = 3; // last shot - reset rockets status for next round

   }
       //   // Check if all ships of player 2 habe been destroyed
    if (isGameOver(1))
    {
      break;
    }
  }
  
  lv_task_handler(); /* let the GUI do its work */
  lv_tick_inc(5);    /* tell LVGL how much time has passed */
  delay(5);          /* let this time pass */

  // Game over, handle the end of the game

  handleGameOver(players_round);
}
