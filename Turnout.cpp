/**********************************************************************

Turnout.cpp, renamed from Accessories.cpp
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#include "DCCpp.h"

#ifdef USE_TURNOUT

#include "Turnout.h"
#include "DCCpp_Uno.h"
// #include "Comm.h"

#ifdef USE_TEXTCOMMAND
#include "TextCommand.h"
#endif

///////////////////////////////////////////////////////////////////////////////

void Turnout::begin(int id, int add, int subAdd)
{
#if defined(USE_TEXTCOMMAND)
#if defined(DCCPP_DEBUG_MODE)
	if (strncmp(EEStore::data.id, EESTORE_ID, sizeof(EESTORE_ID)) != 0)
	{ // check to see that eeStore contains valid DCC++ ID
		DCCPP_INTERFACE.println(F("Turnout::begin() must be called BEFORE DCCpp.begin() !"));
	}
#endif
	if (firstTurnout == NULL)
	{
		firstTurnout = this;
	}
	else if (get(id) == NULL)
	{
		Turnout *tt = firstTurnout;
		while (tt->nextTurnout != NULL)
			tt = tt->nextTurnout;
		tt->nextTurnout = this;
	}
#endif

	this->set(id, add, subAdd);

#ifdef USE_TEXTCOMMAND
	DCCPP_INTERFACE.print("<O>");

#endif
}

///////////////////////////////////////////////////////////////////////////////

void Turnout::set(int id, int add, int subAdd)
{
	this->data.id = id;
	this->data.address = add;
	this->data.subAddress = subAdd;
	this->data.tStatus = 0;
}

///////////////////////////////////////////////////////////////////////////////

void Turnout::activate(int s)
{
	data.tStatus = (s > 0); // if s>0 set turnout=ON, else if zero or negative set turnout=OFF
	DCCpp::mainRegs.setAccessory(this->data.address, this->data.subAddress, this->data.tStatus);
#ifdef USE_TEXTCOMMAND
	DCCPP_INTERFACE.print("<H");
	DCCPP_INTERFACE.print(data.id);
	if (data.tStatus == 0)
		DCCPP_INTERFACE.print(" 0>");
	else
		DCCPP_INTERFACE.print(" 1>");

#endif
}

#if defined(USE_TEXTCOMMAND)
///////////////////////////////////////////////////////////////////////////////

Turnout *Turnout::get(int id)
{
	Turnout *tt;
	for (tt = firstTurnout; tt != NULL && tt->data.id != id; tt = tt->nextTurnout)
		;
	return (tt);
}

///////////////////////////////////////////////////////////////////////////////
void Turnout::remove(int id)
{
	Turnout *tt, *pp;

	for (tt = firstTurnout; tt != NULL && tt->data.id != id; pp = tt, tt = tt->nextTurnout)
		;

	if (tt == NULL)
	{
#ifdef USE_TEXTCOMMAND
		DCCPP_INTERFACE.print("<Xt>");
#endif
		return;
	}

	if (tt == firstTurnout)
		firstTurnout = tt->nextTurnout;
	else
		pp->nextTurnout = tt->nextTurnout;

	free(tt);

#ifdef USE_TEXTCOMMAND
	DCCPP_INTERFACE.print("<O>");
#endif
}

///////////////////////////////////////////////////////////////////////////////

int Turnout::count()
{
	int count = 0;
	Turnout *tt;
	for (tt = firstTurnout; tt != NULL; tt = tt->nextTurnout)
		count++;
	return count;
}

///////////////////////////////////////////////////////////////////////////////

#endif

#if defined(USE_TEXTCOMMAND)
///////////////////////////////////////////////////////////////////////////////

void Turnout::parse(char *c)
{
	// <T address state> : set turnout address to state (0 or 1)

	// parse the command string
	int n, s, m;
	n = strtol(c, &c, 10); // get the turnout number
	if (*c == ' ')
	{
		c++;
	}
	s = strtol(c, &c, 10); // get the state (0 or 1)

	m = n + 3; // simplification de la commande sans EEPROM (Lormedy)

	DCCpp::mainRegs.setAccessory((m >> 2), (m & 3), (s > 0));
	DCCPP_INTERFACE.println("<H " + String(n) + ((s == 0) ? " 0>" : " 1>"));

	//     int n, s, m;
	//     Turnout *t;

	//     switch (sscanf(c, "%d %d %d", &n, &s, &m))
	//     {

	//     case 2: // argument is string with id number of turnout followed by zero (not thrown) or one (thrown)
	//         // simplification de la commande sans EEPROM (Lormedy)
	//         n = n + 3;
	//         DCCpp::mainRegs.setAccessory((n >> 2), (n & 3), (s > 0)); // if s>0 set turnout=ON, else if zero or negative set turnout=OFF
	// #ifdef USE_TEXTCOMMAND
	//         DCCPP_INTERFACE.println("<H " + String(n) + ((s == 0) ? " 0>" : " 1>"));
	// #endif

	//         /*
	//                 t=get(n);
	//                 if (t != NULL)
	//                 {
	//                     if (s < 0)                          // if second argument s is negative, just send the current state of the turnout.
	//                     {
	//                         DCCPP_INTERFACE.print("<H ");
	//                         DCCPP_INTERFACE.print(n);
	//                         if (t->data.tStatus == 0)
	//                             DCCPP_INTERFACE.print(" 0>");
	//                         else
	//                             DCCPP_INTERFACE.print(" 1>");
	//                     }
	//                     else
	//                         t->activate(s);
	//                 }
	//         #ifdef USE_TEXTCOMMAND
	//                 Serial.print(F("<t "));
	//                 Serial.print(n);
	//                 Serial.print(F(" / "));
	//                 Serial.print(s);
	//                 Serial.print(F(" : "));
	//                 Serial.print(m);
	//                 Serial.println(">");
	//               } else {
	//                     DCCPP_INTERFACE.print("<Xt>");
	//               }
	//         #endif
	//         */
	//         break;

	//     case 3: // argument is string with id number of turnout followed by an address and subAddress
	//         create(n, s, m);
	//         break;

	//     case 1: // argument is a string with id number only
	//         remove(n);
	//         break;

	// #ifdef DCCPP_PRINT_DCCPP
	//     case -1: // no arguments
	//         show();
	//         break;
	// #endif
	//     }
}

Turnout *Turnout::create(int id, int add, int subAdd)
{
	Turnout *tt = new Turnout();

	if (tt == NULL)
	{ // problem allocating memory
#ifdef USE_TEXTCOMMAND
		DCCPP_INTERFACE.print("<Xt>");
#endif
		return (tt);
	}

	tt->begin(id, add, subAdd);

	return (tt);
}

#endif // USE_TEXTCOMMAND

#if defined(USE_TEXTCOMMAND)
#ifdef DCCPP_PRINT_DCCPP

///////////////////////////////////////////////////////////////////////////////

void Turnout::show()
{
	Turnout *tt;

	if (firstTurnout == NULL)
	{
		DCCPP_INTERFACE.print("<Xt>");
		return;
	}

	for (tt = firstTurnout; tt != NULL; tt = tt->nextTurnout)
	{
		DCCPP_INTERFACE.print("<H");
		DCCPP_INTERFACE.print(tt->data.id);
		DCCPP_INTERFACE.print(" ");
		DCCPP_INTERFACE.print(tt->data.address);
		DCCPP_INTERFACE.print(" ");
		DCCPP_INTERFACE.print(tt->data.subAddress);
		if (tt->data.tStatus == 0)
			DCCPP_INTERFACE.print(" 0>");
		else
			DCCPP_INTERFACE.print(" 1>");
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////

Turnout *Turnout::firstTurnout = NULL;
#endif

#endif // USE_TURNOUT
