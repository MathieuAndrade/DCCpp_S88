Veuillez lire le fichier "Centrale DCC MEGA2560.pdf" situé dans le répertoire "doc" pour utiliser le logiciel.
Quelques connaissances informatiques sont requises pour installer ce logiciel.
Les fichiers du MEGA ont été compilés avec l'IDE Arduino 1.8.9, 1.8.10. et 1.8.12.
Le projet est livré pour une utilisation série avec un câble USB.

Ethernet : 
Ceux qui souhaitent utiliser l'interface Ethernet devront le préciser aux lignes 137/138 du fichier "DCCppS88.ino" et choisir leur type d'interface Ethernet aux lignes 376/379 du fichier "DCCpp.h"
3 fichiers qui sont dans le répertoire "a_copier_sur_carte_SD", "control.htm", "locos.jso" et "favicon.ico" sont à copier sur une carte SD qui sera insérée dans le réceptacle de la carte interface Ethernet.
Vous pouvez éditer "locos.jso" en respectant sa syntaxe.
Pour ceux qui préfère utiliser les fichiers sur leur PC, dans ce même répertoire, lancez "controlDCC.html" qui utilise le fichier des locos "locos.js" que vous pouvez éditer en respectant sa syntaxe.
L'adresse IP par défaut est "http://192.168.0.200:2560/". L'appel direct à cette adresse depuis un navigateur Internet executera le fichier HTML enregistré sur la carte SD.

Compilez et téléversez ce projet dans votre MEGA2560 et la centrale DCC est prête à répondre aux commandes DCC que vous lui envoyez.
Les schémas de la centrale DCCpp sont sur le site Internet http://lormedy.free.fr/DCCpp.html
Un bouton d'arrêt d'urgence peut s'installer sur le MEGA entre l'entrée A5 et GND.

=======================================================================

Please read the "Centrale DCC MEGA2560.pdf" file to use the software.
Computing knowledge is required to install this software.
The MEGA files were compiled with Arduino IDE 1.8.9, 1.8.10. et 1.8.12.
The sketch is delivered in serial/USB mode.

Ethernet : 
If you are using an Ethernet interface, please select your choice at line 137/138 in the "DCCppS88.ino" file and choose your type of Ethernet interface at lines 376/379 in the "DCCpp.h" file.
3 files located in the directory "a_copier_sur_carte_SD", "control.htm", "locos.jso" et "favicon.ico" must be copied on a SD-card that will be inserted in the Ethernet board socket.
You can edit "locos.jso" by following its syntax.
For those who want to use files on their PC, in the same directory, run "controlDCC.html" which uses locos.js that you can edit by following its syntax.
The default IP address is "http://192.168.0.200:2560/". A direct call to this address from an Internet browser will execute the HTML file stored on the SD card.

Compile et download this sketch into your MEGA2560 and the DCC command station is ready to respond to the DCC commands sent to it.
Schematics of the DCC command station are available on the Internet site http://lormedy.free.fr/DCCpp.html
An emergency stop button can be installed on the MEGA between GND and A5 input.

=======================================================================
