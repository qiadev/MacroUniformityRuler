# $Id: Makefile,v 1.13 2003/09/24 19:19:38 granger Exp $
#

INCLUDES = -I../BSpline

CXX = g++

CXXDEBUGFLAGS = -g -Wall

CXXFLAGS = $(CXXDEBUGFLAGS) $(INCLUDES)

.SUFFIXES: .cpp

OBJS = BSplineF.o BSplineD.o

LIB = libbspline.a

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@

all:: $(LIB)

$(LIB): $(OBJS)
	rm -f $(LIB)
	ar crv $(LIB) $(OBJS)
	ranlib $(LIB)

clean::
	rm -f $(OBJS) $(LIB) core *~

depend::

$(OBJS): BSpline.cpp BandedMatrix.h ../BSpline/BSpline.h
