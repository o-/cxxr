/*CXXR $Id$
 *CXXR
 *CXXR This file is part of CXXR, a project to refactor the R interpreter
 *CXXR into C++.  It may consist in whole or in part of program code and
 *CXXR documentation taken from the R project itself, incorporated into
 *CXXR CXXR (and possibly MODIFIED) under the terms of the GNU General Public
 *CXXR Licence.
 *CXXR 
 *CXXR CXXR is Copyright (C) 2008-10 Andrew R. Runnalls, subject to such other
 *CXXR copyrights and copyright restrictions as may be stated below.
 *CXXR 
 *CXXR CXXR is not part of the R project, and bugs and other issues should
 *CXXR not be reported via r-bugs or other R project channels; instead refer
 *CXXR to the CXXR website.
 *CXXR */

/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 2000, 2001, 2004-6  the R Development Core Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, a copy is available at
 *  http://www.r-project.org/Licenses/
 */


/* We need to know the sizes of certain internal structures */
//#define USE_RINTERNALS

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Defn.h"
#include "CXXR/GCStackRoot.hpp"

using namespace CXXR;

/* A count of the memory used by an object. The following assumptions
   are made.

   1) this is called from user-level, so only some types of objects are
      important.
   2) an object gets charged for all the space allocated on the heap
      and all the nodes specifically due to it, but not for the
      space for its name nor for .Internals it references.
*/
SEXP csduplicated(SEXP x);  /* from unique.c */

static R_size_t objectsize(SEXP s)
{
    int i;
    R_size_t cnt = 0, vcnt = 0;
    SEXP tmp;
    Rboolean isVec = FALSE;

    switch (TYPEOF(s)) {
    case NILSXP:
	return(0);
	break;
    case SYMSXP:
	break;
    case LISTSXP:
    case LANGSXP:
	cnt += objectsize(TAG(s));
	cnt += objectsize(CAR(s));
	cnt += objectsize(CDR(s));
	break;
    case CLOSXP:
	cnt += objectsize(FORMALS(s));
	cnt += objectsize(BODY(s));
	/* no charge for the environment */
	break;
    case ENVSXP:
    case PROMSXP:
    case SPECIALSXP:
    case BUILTINSXP:
	break;
    case CHARSXP:
	vcnt = BYTE2VEC(length(s)+1);
	isVec = TRUE;
	break;
    case LGLSXP:
    case INTSXP:
	vcnt = INT2VEC(length(s));
	isVec = TRUE;
	break;
    case REALSXP:
	vcnt = FLOAT2VEC(length(s));
	isVec = TRUE;
	break;
    case CPLXSXP:
	vcnt = COMPLEX2VEC(length(s));
	isVec = TRUE;
	break;
    case STRSXP:
	{
	    vcnt = PTR2VEC(length(s));
	    GCStackRoot<> dup(csduplicated(s));
	    for (i = 0; i < length(s); i++) {
		tmp = STRING_ELT(s, i);
		if(tmp != NA_STRING && !LOGICAL(dup)[i])
		    cnt += objectsize(tmp);
	    }
	    isVec = TRUE;
	    break;
	}
    case DOTSXP:
    case ANYSXP:
	/* we don't know about these */
	break;
    case VECSXP:
    case EXPRSXP:
    case WEAKREFSXP:
	/* Generic Vector Objects */
	vcnt = PTR2VEC(length(s));
	for (i = 0; i < length(s); i++)
	    cnt += objectsize(VECTOR_ELT(s, i));
	isVec = TRUE;
	break;
    case BCODESXP:
	break;
    case EXTPTRSXP:
	cnt += sizeof(void *);  /* the actual pointer */
	cnt += objectsize(EXTPTR_PROT(s));
	cnt += objectsize(EXTPTR_TAG(s));
	break;
    case RAWSXP:
	vcnt = BYTE2VEC(length(s));
	isVec = TRUE;
	break;
    case S4SXP:
	break;
    default:
	UNIMPLEMENTED_TYPE("object.size", s);
    }
    /* add in node space:
       we need to take into account the rounding up that goes on
       in the node classes. */
    if(isVec) {
	cnt += sizeof(RObject);
	if (vcnt > 16) cnt += 8*vcnt;
	else if (vcnt > 8) cnt += 128;
	else if (vcnt > 6) cnt += 64;
	else if (vcnt > 4) cnt += 48;
	else if (vcnt > 2) cnt += 32;
	else if (vcnt > 1) cnt += 16;
	else if (vcnt > 0) cnt += 8;
    } else cnt += sizeof(RObject);
    /* add in attributes: these are fake for CHARXPs */
    if(TYPEOF(s) != CHARSXP) cnt += objectsize(ATTRIB(s));
    return(cnt);
}


SEXP attribute_hidden do_objectsize(SEXP call, SEXP op, SEXP args, SEXP env)
{
    checkArity(op, args);
    return ScalarReal( double(objectsize(CAR(args))) );
}
