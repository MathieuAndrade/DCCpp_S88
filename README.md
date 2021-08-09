Veuillez lire le fichier "Centrale DCC MEGA2560.pdf" situ� dans le r�pertoire "doc" pour utiliser le logiciel.
Quelques connaissances informatiques sont requises pour installer ce logiciel.
Les fichiers du MEGA ont �t� compil�s avec l'IDE Arduino 1.8.9, 1.8.10. et 1.8.12.
Le projet est livr� pour une utilisation s�rie avec un c�ble USB.

Ethernet : 
Ceux qui souhaitent utiliser l'interface Ethernet devront le pr�ciser aux lignes 137/138 du fichier "DCCppS88.ino" et choisir leur type d'interface Ethernet aux lignes 376/379 du fichier "DCCpp.h"
3 fichiers qui sont dans le r�pertoire "a_copier_sur_carte_SD", "control.htm", "locos.jso" et "favicon.ico" sont � copier sur une carte SD qui sera ins�r�e dans le r�ceptacle de la carte interface Ethernet.
Vous pouvez �diter "locos.jso" en respectant sa syntaxe.
Pour ceux qui pr�f�re utiliser les fichiers sur leur PC, dans ce m�me r�pertoire, lancez "controlDCC.html" qui utilise le fichier des locos "locos.js" que vous pouvez �diter en respectant sa syntaxe.
L'adresse IP par d�faut est "http://192.168.0.200:2560/". L'appel direct � cette adresse depuis un navigateur Internet executera le fichier HTML enregistr� sur la carte SD.

Compilez et t�l�versez ce projet dans votre MEGA2560 et la centrale DCC est pr�te � r�pondre aux commandes DCC que vous lui envoyez.
Les sch�mas de la centrale DCCpp sont sur le site Internet http://lormedy.free.fr/DCCpp.html
Un bouton d'arr�t d'urgence peut s'installer sur le MEGA entre l'entr�e A5 et GND.

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
