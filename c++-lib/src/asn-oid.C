// file: .../c++-lib/src/asn-oid.C   - OBJECT IDENTIFIER
//
//  Mike Sample
//  92/07/02
// Copyright (C) 1992 Michael Sample and the University of British Columbia
//
// This library is free software; you can redistribute it and/or
// modify it provided that this copyright/license information is retained
// in original form.
//
// If you modify this file, you must clearly indicate your changes.
//
// This source code is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
// $Header: /usr/app/odstb/CVS/snacc/c++-lib/src/asn-oid.C,v 1.7 1997/02/28 13:39:46 wan Exp $
// $Log: asn-oid.C,v $
// Revision 1.7  1997/02/28 13:39:46  wan
// Modifications collected for new version 1.3: Bug fixes, tk4.2.
//
// Revision 1.6  1997/02/16 12:32:49  rj
// name lookup of \for' scoping
//
// Revision 1.5  1995/08/17  15:31:14  rj
// set Tcl's errorCode variable
//
// Revision 1.4  1995/07/24  20:25:38  rj
// #if TCL ... #endif wrapped into #if META ... #endif
//
// call constructor with additional pdu and create arguments.
//
// changed `_' to `-' in file names.
//
// Revision 1.3  1994/10/08  04:18:28  rj
// code for meta structures added (provides information about the generated code itself).
//
// code for Tcl interface added (makes use of the above mentioned meta code).
//
// virtual inline functions (the destructor, the Clone() function, BEnc(), BDec() and Print()) moved from inc/*.h to src/*.C because g++ turns every one of them into a static non-inline function in every file where the .h file gets included.
//
// made Print() const (and some other, mainly comparison functions).
//
// several `unsigned long int' turned into `size_t'.
//
// Revision 1.2  1994/08/28  10:01:17  rj
// comment leader fixed.
//
// Revision 1.1  1994/08/28  09:21:06  rj
// first check-in. for a list of changes to the snacc-1.1 distribution please refer to the ChangeLog.

#include "asn-config.h"
#if TCL
#include <strstream>
#endif
#include "asn-len.h"
#include "asn-tag.h"
#include "asn-type.h"
#include "asn-oid.h"
#include "print.h"

using std::endl;

AsnOid::~AsnOid()
{
  delete oid;
}

AsnType *AsnOid::Clone() const
{
  return new AsnOid;
}

// Initializes an AsnOid with a string and it's length.
// The string should hold the encoded OID.
// The string is copied
#ifndef _IBM_ENC_
void AsnOid::Set (const char *encOid, size_t len)
#else
void AsnOid::Set (const char *encOid, const size_t len)
#endif /* _IBM_ENC_ */
{
    if (encOid != oid)
    {
        octetLen = len;
#ifndef _IBM_ENC_
        oid = new char[octetLen];
#else
        oid = (char *) mem_mgr_ptr->Get (octetLen);         /* Guido Grassel, 11.8.93 */
#endif /* _IBM_ENC_ */
        memcpy (oid, encOid, octetLen);
    }
}

// Inits an AsnOid from another OID.
// The oid string is copied.
void AsnOid::Set (const AsnOid &o)
{
    if (&o != this)
    {
        octetLen = o.octetLen;
#ifndef _IBM_ENC_
        oid = new char[octetLen];
#else
        oid = (char *) mem_mgr_ptr->Get (octetLen);         /* Guido Grassel, 11.8.93 */
#endif /* _IBM_ENC_ */
        memcpy (oid, o.oid, octetLen);
    }
}


// Given some arc numbers, an AsnOid is built.
// Set (1, 2, 3, 4, 5, -1, -1, -1, -1, -1, -1) results in
// oid { 1 2 3 4 5 }.  The first negative arc number represnts
// the end of the arc numbers - at least 2 are required.
// The prototype in the AsnOid class provides default -1 parameters
// so you only need to provide the number of arc number in the oid
// as params. (eg Set (1,2,3,4,5))
void AsnOid::Set (unsigned long int a1, unsigned long int a2, long int a3, long int a4, long int a5, long int a6, long int a7, long int a8, long int a9, long int a10, long int a11)
{
    static long int arcNumArr[11];
    static char buf[11*5];  /* make big enough for max oid with 11 arcs*/
    static char *tmpBuf;
    size_t totalLen;
    size_t elmtLen;
    long int tmpArcNum;
    long int headArcNum;

    tmpBuf = buf;

    arcNumArr[0] = a1;
    arcNumArr[1] = a2;
    arcNumArr[2] = a3;
    arcNumArr[3] = a4;
    arcNumArr[4] = a5;
    arcNumArr[5] = a6;
    arcNumArr[6] = a7;
    arcNumArr[7] = a8;
    arcNumArr[8] = a9;
    arcNumArr[9] = a10;
    arcNumArr[10] = a11;

    // munge together first oid arc numbers
    headArcNum = tmpArcNum = (arcNumArr[0] * 40) + arcNumArr[1];

    // figure encoded length for this arc number
    for (elmtLen = 1; (tmpArcNum >>= 7) != 0; elmtLen++)
	;

    // write bytes except the last/least significant of the head arc number
    // more bit is on
    totalLen = elmtLen;
    for (int i = 1; i < elmtLen; i++)
    {
        *(tmpBuf++) = 0x80 | (headArcNum >> ((elmtLen-i)*7));
    }

    // write least significant (more bit is off)
    *(tmpBuf++) = 0x7f & headArcNum;

    // repeat for the rest of the arc numbers
    for (int i = 2; (i < 11) && (arcNumArr[i] > 0); i++)
    {
        tmpArcNum = arcNumArr[i];
        for (elmtLen = 1; (tmpArcNum >>= 7) != 0; elmtLen++)
	    ;
        totalLen += elmtLen;
        tmpArcNum = arcNumArr[i];
        for (int j = 1; j < elmtLen; j++)
        {
            *(tmpBuf++) = 0x80 | (tmpArcNum >> ((elmtLen-j)*7));
        }
        *(tmpBuf++) = 0x7f & tmpArcNum;
    }

#ifndef _IBM_ENC_
    oid = Asn1Alloc (totalLen);
#else
    oid = (char *) mem_mgr_ptr->Get (totalLen);               /* Guido Grassel, 11.8.93 */
#endif /* _IBM_ENC_ */
    memcpy (oid, buf, totalLen);
    octetLen = totalLen;

}  /* AsnOid::Set */



// Like Set except frees old oid value first
#ifndef _IBM_ENC_
void AsnOid::ReSet (const char *encOid, size_t len)
#else
void AsnOid::ReSet (const char *encOid, const size_t len)
#endif /* _IBM_ENC_ */
{
    if (encOid != oid)
    {
#ifndef _IBM_ENC_
        delete oid;
#else
      mem_mgr_ptr->Put ((void *) oid);                /* Guido Grassel, 11.8.93 */
#endif /* _IBM_ENC_ */
        Set (encOid, len);
    }
}

// Like Set except frees old oid value first
void AsnOid::ReSet (const AsnOid &o)
{
    if (&o != this)
    {
#ifndef _IBM_ENC_
        delete oid;
#else
      mem_mgr_ptr->Put ((void *) oid);                /* Guido Grassel, 11.8.93 */
#endif /* _IBM_ENC_ */
        Set (o);
    }
}

void AsnOid::ReSet (unsigned long int a1, unsigned long int a2, long int a3, long int a4, long int a5, long int a6, long int a7, long int a8, long int a9, long int a10, long int a11)
{
#ifndef _IBM_ENC_
    delete oid;
#else
  mem_mgr_ptr->Put ((void *) oid);                /* Guido Grassel, 11.8.93 */
#endif /* _IBM_ENC_ */
    Set (a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}


// returns the number of arc numbers in the OID value
unsigned long int AsnOid::NumArcs() const
{
    int i;
    int numArcs;

    for (numArcs=0, i=0; i < octetLen; )
    {
        // skip octets in this arc num with the 'more' bit set
        for (; (i < octetLen) && (oid[i] & 0x80); i++)
	    ;

        // skip last octet in this arc num (no more bit)
        i++;

        numArcs++;
    }

    // add one to return value because the first two arcs are
    // crunched together into a single one.
    return numArcs +1;

}  /* AsnOid::NumArcs */

int AsnOid::OidEquiv (const AsnOid &o) const
{
    return o.octetLen == octetLen && !strncmp (o.oid, oid, octetLen);
}



// Decodes the content of a BER OBJECT IDENTIFIER value and puts
// the results in this AsnOid object.
void AsnOid::BDecContent (BUF_TYPE b, AsnTag tagId, AsnLen elmtLen, AsnLen &bytesDecoded, ENV_TYPE env)
{
    /* treat like primitive octet string */
    octetLen = elmtLen;
#ifndef _IBM_ENC_
    oid =  Asn1Alloc (elmtLen);
#else
    oid = (char *) mem_mgr_ptr->Get (elmtLen);                /* Guido Grassel, 11.8.93 */
#endif /* _IBM_ENC_ */
    b.CopyOut (oid, elmtLen);

    if (b.ReadError())
    {
        Asn1Error << "BDecOctetString: ERROR - decoded past end of data" << endl;
        longjmp (env, -17);
    }
    bytesDecoded += elmtLen;
} /* AsnOid::BDecContent */

AsnLen AsnOid::BEnc (BUF_TYPE b)
{
    AsnLen l;
    l =  BEncContent (b);
    l += BEncDefLen (b, l);
    l += BEncTag1 (b, UNIV, PRIM, OID_TAG_CODE);
    return l;
}

void AsnOid::BDec (BUF_TYPE b, AsnLen &bytesDecoded, ENV_TYPE env)
{
    AsnLen elmtLen;
    if (BDecTag (b, bytesDecoded, env) != MAKE_TAG_ID (UNIV, PRIM, OID_TAG_CODE))
    {
	Asn1Error << "AsnOid::BDec: ERROR tag on OBJECT IDENTIFIER is wrong." << endl;
	longjmp (env,-57);
    }
    elmtLen = BDecLen (b, bytesDecoded, env);

    BDecContent (b, MAKE_TAG_ID (UNIV, PRIM, OID_TAG_CODE), elmtLen, bytesDecoded, env);
}

AsnLen AsnOid::BEncContent (BUF_TYPE b)
{
    b.PutSegRvs (oid, octetLen);
    return octetLen;
}

// Prints an AsnOid in ASN.1 Value Notation.
// Decodes the oid to get the individual arc numbers
void AsnOid::Print (std::ostream &os) const
{
  unsigned short int firstArcNum;
  unsigned long int arcNum;
  int i;

  // print oid in
  os << "{";

  if (oid)
  {
    // un-munge first two arc numbers
    for (arcNum = 0, i=0; (i < octetLen) && (oid[i] & 0x80); i++)
      arcNum = (arcNum << 7) + (oid[i] & 0x7f);

    arcNum = (arcNum << 7) + (oid[i] & 0x7f);
    i++;
    firstArcNum = arcNum/40;
    if (firstArcNum > 2)
      firstArcNum = 2;

    os << firstArcNum  << " " << arcNum - (firstArcNum * 40);

    for (; i < octetLen; )
    {
      for (arcNum = 0; (i < octetLen) && (oid[i] & 0x80); i++)
	arcNum = (arcNum << 7) + (oid[i] & 0x7f);

      arcNum = (arcNum << 7) + (oid[i] & 0x7f);
      i++;
      os << " " << arcNum;
    }
  }
  else
    os << "-- void --";

  os << "}";

} // AsnOid::Print

#if META

const AsnOidTypeDesc AsnOid::_desc (NULL, NULL, false, AsnTypeDesc::OBJECT_IDENTIFIER, NULL);

const AsnTypeDesc *AsnOid::_getdesc() const
{
  return &_desc;
}

#if TCL

int AsnOid::TclGetVal (Tcl_Interp *interp) const
{
  if (oid)
  {
    strstream buf;
    buf << *this;
    buf.str()[strlen(buf.str())-1] = '\0';			// chop the trailing '}'
    Tcl_SetResult (interp, buf.str()+1, TCL_VOLATILE);	// copy without leading '{'
  }
  return TCL_OK;
}

int AsnOid::TclSetVal (Tcl_Interp *interp, const char *valstr)
{
  if (!*valstr)
  {
    delete oid;
    oid = NULL;
    octetLen = 0;
    return TCL_OK;
  }

  Args arc;
  if (Tcl_SplitList (interp, (char*)valstr, &arc.c, &arc.v) != TCL_OK)
    return TCL_ERROR;
  if (arc.c < 2)
  {
    Tcl_AppendResult (interp, "oid arc must contain at least two numbers", NULL);
    Tcl_SetErrorCode (interp, "SNACC", "ILLARC", "<2", NULL);
    return TCL_ERROR;
  }
  if (arc.c > 11)
  {
    Tcl_AppendResult (interp, "snacc limits oid arcs to no more than 11 numbers", NULL);
    Tcl_SetErrorCode (interp, "SNACC", "ILLARC", ">11", NULL);
    return TCL_ERROR;
  }

  int na[11], i;
  for (i=0; i<arc.c; i++)
    if (Tcl_GetInt (interp, arc.v[i], na+i) != TCL_OK)
      return TCL_ERROR;
  while (i < 11)
    na[i++] = -1;

  ReSet (na[0], na[1], na[2], na[3], na[4], na[5], na[6], na[7], na[8], na[9], na[10]);

  return TCL_OK;
}

#endif /* TCL */
#endif /* META */
