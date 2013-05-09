
EXE_NAME = volumerender/cxvolume

CONFIG = RESEARCH

SYSTEM = LINUX


ifeq ($(SYSTEM), LINUX)
	CPPFLAGS = -g -c -Wall
	CXX=g++
	#NETCDF = $(HOME)/local/netcdf-3.6.2
	#FLTK = $(HOME)/local/lib64/fltk
        #FLTK = $(HOME)/local/fltk-1.1.7	
        #GLUT = $(HOME)/local/lib64/glut
	#GLEW = $(HOME)/local/lib64/glew
		
	#INCLUDE = -I/usr/X11R6/include -I$(FLTK)/include -I$(GLEW)/include -I$(NETCDF)/include
	INCLUDE = -I/usr/X11R6/include -I/usr/include/netcdf-3
	#LIB = -L$(FLTK)/lib -L$(GLUT)/lib -L$(GLEW)/lib -L/usr/X11R6/lib64 -I$(NETCDF)/include
	LIB = -L/usr/lib64 -L$(GLUT)/lib -L$(GLEW)/lib -L/usr/X11R6/lib64 -I$(NETCDF)/include
	#LDFLAGS = -lXmu -lXi -lfltk -lfltk_gl -lGL -lGLU -lglut -lGLEW -ludunits -lm -lnetcdf
	LDFLAGS = -lXmu -lXi -lfltk -lfltk_gl -lGL -lGLU -lglut -lGLEW -lm -lnetcdf
endif

ifeq ($(SYSTEM), MAC_X11)
	CPPFLAGS = -g -c -Wall
	CXX=g++
	FLTK_INC = $(HOME)/local/lib32/fltk/include
	FLTK_LIB = $(HOME)/local/lib32/fltk/lib
	GLUT_INC = /opt/local/include
	GLUT_LIB = /opt/local/lib
	GLEW_INC = /opt/local/include
	GLEW_LIB = /opt/local/lib
	OPENGL_INC = /usr/X11R6/include
	OPENGL_LIB = /usr/X11R6/lib	
	INCLUDE = -I$(FLTK_INC) -I$(GLUT_INC) -I$(GLEW_INC) -I$(OPENGL_INC)
	LIB = -L$(FLTK_LIB) -L$(GLUT_LIB) -L$(GLEW_LIB) -L$(OPENGL_LIB)
	LDFLAGS = -lfltk -lfltk_gl -lGL -lGLU -lglut -lGLEW -lXmu -lX11 -static
endif

ifeq ($(CONFIG), RESEARCH)
	#CPPFLAGS  += -DMULTI_VARIABLES
	#CPPFLAGS += _DWIN_HISTORY
	#CPPFLAGS += -DGENERIC_DATA
	#CPPFLAGS += -DDPROC_DATA
	CPPFLAGS += -DWIN_SLICE
	#CPPFLAGS += -DTIMING
	CPPFLAGS += -DSAVE_SINGLE_IMAGE
	CPPFLAGS += -DSAVE_IMAGES
	#CPPFLAGS += -DSHOWOPACITY
	#CPPFLAGS += -DDRAW_LARGEIMAGE
	CPPFLAGS += -DROTATION
	CPPFLAGS += -DCUT
	CPPFLAGS += -DSHIFT
	CPPFLAGS += -DREAD_HEADFILE
	CPPFLAGS += -DQUANTIZATION	
	#CPPFLAGS += -DHISTOGRAM
	#CPPFLAGS += -DSUPERNOVA
	CPPFLAGS += -DCLIMATE
	CPPFLAGS += -DORTHO
        CPPFLAGS += -DWIN_NETCDF
        CPPFLAGS += -DLINUX
        CPPFLAGS += -DSHOW_CLUSTER
        CPPFLAGS += -DCHANGES
	CPPFLAGS += -DSV_CORRELATION
	        
	#CPPFLAGS += -DDRAWIMPORTANCE
endif

ifeq ($(CONFIG), PUBLIC)
	CPPFLAGS += -DPUBLIC_VERSION
endif

OBJ_DIR = .
CPPFILES = main.cpp clusterview2d.cpp cxparameters.cpp cx3dvector.cpp cxvolume.cpp winvisview.cpp \
           winvismain.cpp shader.cpp \
           KCtree.cpp KCutil.cpp KM_ANN.cpp KMcenters.cpp KMdata.cpp KMeans.cpp KMfilterCenters.cpp \
           KMlocal.cpp KMrand.cpp KMterm.cpp kmlsample.cpp pm_hist.cpp \
           cxcolor.cpp mydragbutton.cpp wintinydraw.cpp wincolormap.cpp cxmatrix_c.cpp \
           winctrlmain.cpp winctrlview.cpp winctrllight.cpp winsliceview.cpp \
           winctrlhistory.cpp image.cpp winhistogram.cpp netcdf_read.cpp \
           winctrlnetcdf.cpp winctrlnetcdfview.cpp parcrd.cpp winctrlca.cpp marchingsquares.cpp rectdraw.cpp framebuffer.cpp \
           Histogram1D.cpp OpacityDisp.cpp TransferFunc1D.cpp

OBJECT_FILES = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(CPPFILES))

all: $(OBJECT_FILES)	
	$(CXX) -o ${EXE_NAME} $(OBJECT_FILES) $(LIB) $(LDFLAGS)


$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(INCLUDE) $(ENV) -o $@ $<

include .depend

.depend:
	g++ -g -MM -E $(CPPFLAGS) $(INCLUDE) $(CPPFILES) > .depend	

clean:
	rm -f core.* $(OBJ_DIR)/*.o $(EXE_NAME) .depend


