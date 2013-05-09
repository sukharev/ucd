# -*- coding: utf-8 -*-


#  $Id: rymodtran.py 108 2012-11-11 18:31:15Z neliswillers@gmail.com $
#  $HeadURL: http://pyradi.googlecode.com/svn/trunk/rymodtran.py $

################################################################
# The contents of this file are subject to the Mozilla Public License
# Version 1.1 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/

# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
# License for the specific language governing rights and limitations
# under the License.

# The Original Code is part of the PyRadi toolkit.

# The Initial Developer of the Original Code is AE Mudau,
# Portions created by AE Mudau are Copyright (C) 2012
# All Rights Reserved.

# Contributor(s):  CJ Willers, PJ Smit.
################################################################
"""
This module provides rudimentary MODTRAN file reading.

See the __main__ function for examples of use.
"""

#prepare so long for Python 3
from __future__ import division
from __future__ import print_function
#from __future__ import unicode_literals

__version__= "$Revision: 108 $"
__author__= 'pyradi team'
__all__= ['fixHeaders', 'loadtape7','fixHeadersList', 'savetape7data']

import numpy as np
from string import maketrans
import StringIO
import ryplot
import ryutils
import re
import math

class CardDeck:
    # mu = -1.0 + 2.0 * y / (float(RES_MU) - 1.0);
    # muS = mod(x, float(RES_MU_S)) / (float(RES_MU_S) - 1.0);
    # muS = -0.2 + muS * 1.2;
    # nu = -1.0 + floor(x / float(RES_MU_S)) / (float(RES_NU) - 1.0) * 2.0;
    #RES_R = 10, RES_MU = 128, RES_MU_S = 32, RES_NU = 8;
    def __init__(self, in_fname, bWrite, mu_s, nu, mu, r):
        self.test = True;
        self._res_mu_s = mu_s;    #cos solar zenith
        self._res_nu = nu;        #nuFromAzimuth(self, ang_azimuth, ang_mu, ang_mu_s):
        self._res_mu = mu;        #cos OBSZEN 
        self._res_r = r;          #H1ALT + radius of earth
        #the minimum altitude H1ALT 0.16075426526368 meters, and the final altitude (25.821244410585 m)
        self._fname = in_fname;
        self._numLOSs = 0;
        self._wv_id = [-1,-1,-1];
        self._startWL = 0;
	self._endWL = 0;
	self._intervalWL = 0
	self._AtmDepth = 80.0; #34 #25.821244; #98.00;
	self._Rg = 6371.23;#6360.0;
	self._Rt = 6371.23 + self._AtmDepth + 1.0;#6437.23;#6420.0;
	
	self._bNonlinear = True;

    def getH1ALT(self, ri):
    	#part = (25.821244 - 0.1607542)/self._res_r
    	part = (self._AtmDepth - 0.1607542)/self._res_r
    	r = 0.1607542 + part*ri
    	#part = (6.420 - 6.371)/self._res_r
    	#r = 6.371 + part*ri
    	return r
    
    def calcHALT2(self, mu, mu_s, nu, H1ALT):
    	alt = 0;
    	LENN = 0;
    	r = H1ALT + self._Rg;
    	p = r * r * (mu * mu - 1.0) + self._Rg * self._Rg;
    	
    	t = -1;
    	if p >= 0:
    		t = -r * mu - math.sqrt(p);
    		
    	if t>=0.0:
    		alt = 0.000;
    		LENN = -1;
    		print("LENN: -1");
    	else:
    		#determin LENN parameter
    		LENN = 1;
    		alt = 100.0;
    		print("LENN: 1");
    	
    	return [alt,LENN]; 
    	
    def readtape5(self):
    	print("reading tape5 file... " + self._fname)
    	#numWavelen = 152; # number of Wavelength    CARD 4 -> ((700 - 400) / 2) + 2
    	filename = self._fname;
    	infile = open(filename, 'r')
	idata = {}
	lines = infile.readlines()#.strip()
	infile.close()
	
	ITYPE = int(lines[0][9])
	#print("ITYPE : "+str(ITYPE))
	IEMSCT = int(lines[0][14])
	IMULT = int(lines[0][19])
	#print("ITYPE : "+str(ITYPE) + ", IMULT : " + str(IMULT) + ", IEMSCT : "+str(IEMSCT))
	#print('filename={0}, IEMSCT={1}'.format(filename,IEMSCT))
	
	#skip the first few rows that contains tape5 information and leave the
	#header for the different components of transimissions.
	#find the end of the header.
	headline = 0
	
	self.readcard1(lines[headline]); #CARD 1
	self.readcard3(lines[headline+4]); # CARD 3
	#print(lines[headline+5]);
	self.readcard3a2(lines[headline+6]); # CARD 3A2
	self.readcard4(lines[headline+7]); # CARD 4
	#print(first_col);
	
	
	#some files has only a single text column head, while others have two
	# find out what the case is for this file and concatenate if necessary
	deltaHead = 0
	
	
		
	startpos = 0;
	s_ext = 'start';
	coldata = ''
	
	headline = headline + 8;
	final_result = []; #np.ndarray( [] );
	count = 0;
	while count < self._numLOSs-1:
		#print("startpos: " + str(startpos) + ", headline: " + str(headline));
		#s = ' '.join(colHead) + '\n'
		s_ext = ''.join(lines[headline+count:headline+1+count]);
		self.readcard3d(s_ext);
		if str(s_ext) == '':
	    		break;
		#s = s + ''.join(lines[headline+1+deltaHead+startpos:headline+1+deltaHead+startpos+self._numLOSs-1])
		#data = re.split(r'            ', s);
		#s = " 0.000 ".join(str(x).strip() for x in data);
		#lines1 = np.ndfromtxt(StringIO.StringIO(s), dtype=None,  names=True)

		#final_result += lines1[first_col].tolist();
		count = count + 1;
	return "mainloop";
	
    def writetape5(self, fname, SOL_azy, h1alt_id):
        filename = fname;
        outfile = open(filename, 'w')
        line = " ";
        numLOSs = self._res_nu * self._res_mu; #73;
    	self.writecard1(line, outfile, numLOSs);
    	self.writecard1_2ext(outfile);
    	first = True;
    	#print("self._res_mu: " + str(self._res_mu) + ", c_mu: " + str(math.acos(-1.0 + 2.0 * 64 / ((self._res_mu) - 1.0))));
    	#print("self._res_nu: " + str(self._res_nu) + ", c_azim: " + str(math.acos(-1.0 + 2.0 * 4 / (float(self._res_nu) - 1.0))));
    	
    	#//TODO: replace mu_s with mod(X, float(RES_MU_S)) / (float(RES_MU_S) - 1.0);
    	#// where X is in [0..res_nu*_res_mu_s]
    	
    	sx, sy = divmod(SOL_azy, self._res_mu_s);
    	mu_s= float(sy) / ((self._res_mu_s) - 1.0);
    	mu_s= -0.2 + mu_s * 1.2;
    	#mu_s= -1.0 + 2.0 * SOL_azy / ((self._res_mu_s) - 1.0);
	a_mu_s = math.acos(mu_s) * 180 / math.pi;
	if a_mu_s == 180.0:
		a_mu_s = 179.9;
		
	H1ALT = self.getH1ALT(h1alt_id)
    	for x in range(0, self._res_nu):
    		nu = -1.0 + 2.0 * x / (float(self._res_nu) - 1.0);    			
		#cos_azim = self.SolAzimuth(nu, mu, mu_s);
		##cos_azim = nu
    		#azim = math.acos(cos_azim) * 180 / math.pi;
    		
    		
    		for y in range(0,self._res_mu):
    			mu = -1.0 + 2.0 * y / ((self._res_mu) - 1.0);
    			#print("cos_azim: " + str(cos_azim) + ", muS, mu, nu: " + str(mu_s) + ", " + str(mu) + ", " + str(nu)+"\n");
				
    		 	if self._bNonlinear == True:
    		 		nu_old = nu;
    		 		mu_s_old = mu_s;
    		 		mu_old = mu;
    				[mu, mu_s, nu] = self.inscatter_nonlinear(h1alt_id, SOL_azy, x, y);
    				#print("mu_old: " + str(mu_old) + ", mu: " + str(mu) + "\n");
    				cos_azim = self.SolAzimuth(nu, mu, mu_s);
    				#cos_azim = nu;
    				#mu = mu_old;
    				#mu_s = mu_old;
    			else:
    				cos_azim = self.SolAzimuth(nu, mu, mu_s);	
    			a_mu_s = math.acos(mu_s) * 180 / math.pi;
    			azim = math.acos(cos_azim) * 180 / math.pi;	
			a_mu = math.acos(mu) * 180 / math.pi;
			if a_mu == 180.0:
				a_mu = 179.9;
			if a_mu_s == 180.0:
				a_mu_s = 179.9;
		
			# make sure that HALT2 is not larger than HALT1 when LOS hits the ground
			# TODO: come up with a better method then the simple one below:
			
			# determine HALT2 programatically
			# HALT2 = slf.calcHALT2(mu, mu_s, nu);
			#HALT2 = 100.0;
			#LENN = -1;
			[HALT2, LENN] = self.calcHALT2(mu, mu_s, nu, H1ALT);
			#if a_mu > 90.0:
    			#        HALT2 = 0.1607542;	
    			if first == True:
				#line = "0.5000000 {:>4.4f} {:>10f}           6371.23000".format(HALT2,a_mu);
				if LENN > -1:
					print("1) HALT2: " + str(HALT2) + ", LENN: " + str(LENN) + "\n"); 
					line = "{:>10f} {:>4.4f} {:>10f}           6371.23000 {:>5d}".format(H1ALT, HALT2,a_mu, int(LENN));
					print("LINE: " + line)
				else:
					line = "{:>10f} {:>4.4f} {:>10f}           6371.23000".format(H1ALT, HALT2,a_mu);
				self.writecard3(line, outfile);
				self.writecard3a1(outfile);
				#line = "0.00000  40.00000";
				line = "0.00000 {:>4.5f}".format(a_mu_s);
				self.writecard3a2(line, outfile);
				#line = "440.000 660.000 3";
				line = "340.000 760.000 3";
    				self.writecard4(line, outfile);
    				first = False;
    			else:
    				if LENN > -1:
    					print("2) HALT2: " + str(HALT2) + ", LENN: " + str(LENN) + "\n");
    					line = "{:>10f} {:>4.4f} {:>3.5f} {:>3.4f} {:>5d}".format(H1ALT, HALT2, a_mu, azim, int(LENN));
				else:
    					line = "{:>10f} {:>4.4f} {:>3.5f} {:>3.4f}".format(H1ALT, HALT2, a_mu, azim);
    				self.writecard3d(line, outfile);
    	#line = "2  300.000\n"
	#self.writecardX(line, outfile);
	#line = "DATA/spec_alb.dat\n"
    	#self.writecardX(line, outfile);
    	#line = "constant, 10%\n"
    	#for z in range(0, self._res_mu*self._res_nu+2):
    	#	self.writecardX(line, outfile);
    	line = "    0\n";
    	self.writecardX(line, outfile);
    	outfile.close()

    def readcard1(self, line):
    	arr = line.split();
    	self._numLOSs = abs(int(arr[2]));
    	#print("Number of LOCs = " + str(arr[2]));
    def writecard1(self, line, fn, numLOSs):
    	#numLOSs = -73;
    	# with 0.1000 SURREF surface reflectance
    	#n = "M   6{:>5}    2    1    0    0    0    0    0    0    0    0    1    .000 0.1000\n".format(str(-numLOSs));
    	n = "M   6{:>5}    2    1    0    0    0    0    0    0    0    0    1    .000       \n".format(str(-numLOSs));
 	fn.write(n);
    def writecardX(self,line, fn):
    	fn.write(line);
    def writecard1_2ext(self, fn):
    	n1 = "TT   8   0   380.000  1.000000           F T F   F\n"
	n2 ="15_2009\n"
	# last parameter is GRNALTR - ground altitude - 0.00000
    	# CARD 2: suppose to produce Rayleigh scattering only
    	n3 = "    0    0    0    0    0    0  50.00000    .00000    .00000    .00000   0.00000\n"
    	fn.write(n1);
    	fn.write(n2);
    	fn.write(n3);
    def readcard3(self, line):
    	arr = line.split();
    	#print("Radius of Earth = " + str(arr[3]));
    def writecard3(self, line, fn):
    	arr = line.split();
    	#print(str(arr));
    	if len(arr) == 5:
		n = '{:>10}{:>10}{:>10}{:>10}{:>10}{:>10}{:>5}{:>10}\n'.format(arr[0],arr[1],arr[2],' ',' ',arr[3],arr[4],' ');
    	else:
    		n = '{:>10}{:>10}{:>10}{:>10}{:>10}{:>10}{:>10}\n'.format(arr[0],arr[1],arr[2],' ',' ',arr[3],' ');
    	fn.write(n);
    def writecard3a1(self, fn):
    	n = "    2    2    0    0\n";
    	fn.write(n);
    def readcard3d(self, line):
    	arr = line.split();
    	#print("H1ALT (r) = " + str(arr[0]) + ", OBSZEN (mu) = " + str(arr[2]) + ", AZ_INP = " + str(arr[3]));
    def writecard3d(self, line, fn):
    	arr = line.split();
    	#print(str(arr));
    	if len(arr) == 5:
    		n = '{:>10}{:>10}{:>10}{:>10}{:>10}{:>10}{:>5}\n'.format(arr[0],arr[1],arr[2],' ',' ',arr[3],arr[4]);
    	else:
    		n = '{:>10}{:>10}{:>10}{:>10}{:>10}{:>10}\n'.format(arr[0],arr[1],arr[2],' ',' ',arr[3]);
    	fn.write(n);
    def readcard3a2(self, line):
    	arr = line.split();
    	#print("SOLAR ZENITH AT H1ALT (mu_s) = " + str(arr[1]));
    	#return "readcard3a2";
    def writecard3a2(self, line, fn):
        arr = line.split();
    	n = '{:>10}{:>10}                                          30.00000\n'.format(arr[0],arr[1]);
    	fn.write(n);
    def readcard4(self, line):
    	arr = line.split();
	self._startWL = float(arr[0]);
	self._endWL = float(arr[1]);
	self._intervalWL = int(arr[2]);
	# need of three wavelength 680(660),550,440
    	#print("startWL = " + str(self._startWL) + ", endWL = " + str(self._endWL) + ", intervalWL = " + str(self._intervalWL));
    	
    def card4_arr(self):		
    	arr = [self._startWL + val*self._intervalWL for val in range(int((self._endWL + self._intervalWL - self._startWL) / self._intervalWL))]
	return arr;
    def writecard4(self, line, fn):
    	arr = line.split();
    	n = '{:>10}{:>10}{:>10}         2RM $      NGAA\n'.format(arr[0],arr[1],arr[2]);
    	fn.write(n)
    @staticmethod
    def NuFromAzimuth(ang_azimuth, mu, mu_s):
        #mu = math.cos(ang_mu);
        #mu_s = math.cos(ang_mu_s);
        nu = mu*mu_s + math.sqrt( (1-mu**2)*(1-mu_s**2) )*math.cos(ang_azimuth);
        return nu;
    @staticmethod
    def SolAzimuth(nu, mu, mu_s):
    	smallNum = 0.0; #0.01;
        #bMuInterval = (mu > 1.0 and mu < 1.0 + smallNum) or  (mu < -1.0 and mu > -1.0 - smallNum);
        #bMuSInterval  = (mu_s > 1.0 and mu_s < 1.0 + smallNum) or  (mu_s < -1.0 and mu_s > -1.0 - smallNum);
     	c_ang_az = -1.0;#0.5;
     	if mu < 1.0-smallNum and mu_s < 1.0-smallNum and mu > -1.0+smallNum and mu_s > -1.0+smallNum: #and bMuInterval != True and bMuSInterval != True:  
    		c_ang_az = (nu - mu*mu_s) / math.sqrt( (1-mu**2)*(1-mu_s**2) )
    		# for mu == 1 || mus == 1 pick any c_ang_az
    		#ang_az = math.acos(c_ang_az);
    		
    	#if (mu <= -1.0 or mu >= 1.0) and (c_ang_az < -1.0 or c_ang_az > 1.0):
    	#	print("ERROR:\n");
    	if c_ang_az < -1.0 and mu < 1.0 and mu > -1.0:
    		print("-1:  cos_azim: " + str(c_ang_az) + ", muS, mu, nu: " + str(mu_s) + ", " + str(mu) + ", " + str(nu)+"\n");
    		c_ang_az = -1.0;#0.5;#-1.0;
    	elif c_ang_az > 1.0 and mu < 1.0 and mu > -1.0:
    		print("+1:  cos_azim: " + str(c_ang_az) + ", muS, mu, nu: " + str(mu_s) + ", " + str(mu) + ", " + str(nu)+"\n");
    		c_ang_az = 1.0;#0.5;#1.0;
    		
        return c_ang_az;
    def WavelenToInd(self, arr, wv1, wv2, wv3):
        n = [wv1,wv2,wv3];
        #a = [-1,-1,-1];
        for j in range(3):
            for ix in range(len(arr)):
               if float(arr[ix]) == float(n[j]):
                  #a[j] = ix;
                  self._wv_id[j] = ix;
                  break;
               #elif int(arr[ix]) > n and int(arr[old_ix]) < n:
               #   return ix;
               #old_ix = ix; 
        #return a;
    
    def inscatter_nonlinear(self, r_id, SOL_azy, x, y):
    	layer = r_id;
    	top = self._Rt;
    	r = layer / (self._res_r - 1.0);
	r = r * r;
	
	part = (self._AtmDepth - 0.1607542)/self._res_r
	top = self._Rg + 0.1607542 + part*self._res_r
    	#r_ = 0.1607542 + part*r_id
    	#r_ = r_ * r_;
    	
	r1 = math.sqrt(self._Rg * self._Rg + r * (top * top - self._Rg * self._Rg));
	r1 = r1 + (0.01 if layer == 0 else (-0.001 if layer == self._res_r - 1 else 0.0));	
	dmin = top - r1;
    	dmax = math.sqrt(r1 * r1 - self._Rg * self._Rg) + math.sqrt(top * top - self._Rg * self._Rg);
    	dminp = r1 - self._Rg;
   	dmaxp = math.sqrt(r1 * r1 - self._Rg * self._Rg);
    	if y < float(self._res_mu) / 2.0 :
	     d = 1.0 - y / (float(self._res_mu) / 2.0 - 1.0);
	     d = min(max(dminp, d * dmaxp), dmaxp * 0.999);
	     mu = (self._Rg * self._Rg - r1 * r1 - d * d) / (2.0 * r1 * d);
	     mu = min(mu, - math.sqrt(1.0 - (self._Rg / r1) * (self._Rg / r1)) - 0.001);
    	else:
	     d = (y - float(self._res_mu) / 2.0) / (float(self._res_mu) / 2.0 - 1.0);
	     d = min(max(dmin, d * dmax), dmax * 0.999);
	     print("DEBUG: r_id: " + str(r_id) + ", r1: " + str(r1) + ", y: " + str(y) + ", d: " + str(d));
	     print("DEBUG: dmin: " + str(dmin) + ", dmax: " + str(dmax) + ", dminp: " + str(dminp) + ", dmaxp: " + str(dmaxp));
	     mu = (top * top - r1 * r1 - d * d) / (2.0 * r1 * d);
	if mu < -1.0:
		mu = -0.999;
	if mu > 1.0:
		mu = 0.999;
	
	sx, sy = divmod(SOL_azy, self._res_mu_s);
    	mu_s = float(sy) / ((self._res_mu_s) - 1.0);
    	#// paper formula
    	#//muS = -(0.6 + log(1.0 - muS * (1.0 -  exp(-3.6)))) / 3.0; 
    	#// better formula
    	mu_s = math.tan((2.0 * mu_s - 1.0 + 0.26) * 1.1) / math.tan(1.26 * 1.1);
    	nu = -1.0 + 2.0 * x / (float(self._res_nu) - 1.0); 
    	
    	#cos_azim = self.SolAzimuth(nu, mu, mu_s);
    	#nu = -1.0 + floor(x / float(self.res_mu_s)) / (float(self._res_nu) - 1.0) * 2.0;
    	print("===>muS, mu, nu: " + str(mu_s) + ", " + str(mu) + ", " + str(nu)+"\n");
    	return ( mu, mu_s, nu);
    	
    
    ##############################################################################
    ##http://stackoverflow.com/questions/1324067/how-do-i-get-str-translate-to-work-with-unicode-strings
    # RES_MU_S * RES_NU, RES_MU, RES_R
    # TODO: right now nu is not nu but cosAzimuth THIS NEEDS TO CHANGE!!!!
    def writeppm(self, ppmformat, arr, res_mu_s, res_nu, res_mu, r_id): 
        assert ppmformat in ['P3', 'P6'], 'Format wrong'
        filename = "ppm/pas_python_" + str(r_id) + ".ppm"
        f = open(filename, 'wb')
        
        filenamet = "ppm/pas_python_" + str(r_id) + ".txt"
        ft = open(filenamet, 'w')
        
        magic = ppmformat + '\n'
        comment = '# generated from Bitmap.writeppm\n'
        maxval = 256; #max(max(max(bit) for bit in row) for row in self.map)
        #assert ppmformat == 'P3' or 0 <= maxval < 256, 'R,G,B must fit in a byte'
        if ppmformat == 'P6':
            fwrite1 = lambda s: f.write(s); #bytes(s));#, 'UTF-8'))
            maxval = 255
        else:
            fwrite = f.write
            numsize=len(str(maxval))
        fwrite1(magic)
        #fwrite(comment)
        width = 0;
        height = 0;
        maxval = 255;
        fwrite1('%i %i\n%i\n' % (res_mu_s*res_nu, res_mu, maxval))
        #fwrite('%i %i\n' % (res_mu_s*res_nu, res_mu))
        
        out_arr = [];
        
        #muS = mod(x, float(RES_MU_S)) / (float(RES_MU_S) - 1.0);
        #muS = -0.2 + muS * 1.2;
        #nu = -1.0 + floor(x / float(RES_MU_S)) / (float(RES_NU) - 1.0) * 2.0;
        
        for z in range(0,res_mu_s):
        	#sx, sy = divmod(z, res_mu_s);
    		#mu_s= sy / (float(res_mu_s) - 1.0);
    		mu_s= float(z) / (float(res_mu_s) - 1.0);
        	mu_s= -0.2 + mu_s * 1.2;
        	a = arr[z];
        	a_1 = a[0];
        	
        	
        	    
        	for y in range(0, res_nu):
        	    for x in range(0,res_mu):
        	    	mu = -1.0 + 2.0 * x / ((res_mu) - 1.0);
        	        
        	        #print(str(len(a_1)))
        	        if len(a_1) > 0:
        	    		c_azim = -1.0 + 2.0 * y / (float(res_nu) - 1.0);    			
    				azim = math.acos(c_azim) * 180 / math.pi;
        	        	print("mu_s: " + str(mu_s) + ", mu: " + str(mu) + ", azim: " + str(azim));
        	        	real_nu = CardDeck.NuFromAzimuth(azim, mu, mu_s);
        	        	real_nu_id = int((real_nu + 1.0) * (float(res_nu)-1.0)/2);
        	        	#print("real_nu: " + str(real_nu) + ", azim: " + str(azim));
        	        	
        	        	#print("===> " + "res_mu: " + str(res_mu) + ", " + str(y) + " * " + str(res_nu) +" * " + str(x));
        	        	#if len(a_1) >= y*res_nu+x and a_1[y*res_nu+x] != []:
        	        	
    				#b,g,r = a_1[x*res_nu+y];
    				b,g,r = a_1[y*res_mu+x];
    			
    				if math.isnan(b):
    					b = 0.0;
    				if math.isnan(g):
    					g = 0.0;
    				if math.isnan(r):
    					r = 0.0;
    			
    				if r > 1.0:
    					r = 1.0;
    				if g > 1.0:
    					g = 1.0;
    				if b > 1.0:
    					b = 1.0;
    			
    				#################################################################
    				#_STORAGE_TEST_
    				#################################################################
    				#if self._bNonlinear == True:
    				#	nu = c_azim;
    				#    	nu_old = nu;
    				#    	mu_s_old = mu_s;
    				#    	mu_old = mu;
    				#    	[mu, mu_s, nu] = self.inscatter_nonlinear(r_id, z, y, x);
    				#    	#print("mu_old: " + str(mu_old) + ", mu: " + str(mu) + "\n");
    				#    	cos_azim = self.SolAzimuth(nu, mu, mu_s);
    				#else:
    				#	nu = c_azim;
        			#	cos_azim = self.SolAzimuth(nu, mu, mu_s);
    				#b = cos_azim; #real_nu;
    				#g = mu_s;
    				#r = mu;
    				##ft.write('[%d][%d] = %f,%f,%f\n' % (z*res_nu+y,x,r,g,b));
        			#################################################################
    				out_arr.append([b,g,r, y, real_nu_id]);
    				#r, g, b = self.get(w, h)
    				#r = pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim]*(float)256;  // red
    				#g = pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim+1]*(float)256;  // green
    				#b = pTexBuffer[d*texX*texY*sizeDim + i*texY*sizeDim + j*sizeDim+2]*(float)256;  // blue 
    				#fwrite1('%c%c%c' % (int(r*255), int(g*255), int(b*255)));
    			else:
    				out_arr.append([0,0,0,-1,-1]);
    				#fwrite1('%c%c%c' % (0, 0, 0))
        size = res_mu_s* res_nu * res_mu;
        trans_arr = size*[None]
        count = 0;
        for x in range(0,res_mu):
            for y in range(0, res_nu):
                for z in range(0,res_mu_s):
           	    b,g,r,old_nu,new_nu = out_arr[z*res_nu*res_mu+y*res_mu+x]; 
           	    trans_arr[count] = out_arr[z*res_nu*res_mu+y*res_mu+x];#new_nu];
           	    #trans_arr[count] = out_arr[z*res_nu*res_mu+res_mu_s*new_nu + res_nu*res_mu_s*x];
           	    if old_nu != -1:
           	       print("curr_nu: " + str(y) + ", old_nu: " + str(old_nu) + ", new_nu: " + str(new_nu));
           	    count += 1;     
           	    	
        #for y in range(0, res_mu):
        #	for x in range(0,res_mu_s*res_nu):	
        #	    #b,g,r = out_arr[x*res_mu+y];
        #	    b,g,r, o, n = trans_arr[y*res_mu_s*res_nu+x];
        #	    #fwrite1('%c%c%c' % (int(r*255), int(g*255), int(b*255)));
        #	    ft.write('[%d][%d] = %f,%f,%f\n' % (x,z*res_nu+y,r,g,b));
        
        
        for x in range(0,res_mu_s*res_nu):
            for y in range(0, res_mu):
        	#b,g,r = out_arr[x*res_mu+y];
        	b,g,r, o, n = trans_arr[x*res_mu+y];
        	#ft.write('[%d][%d] = %d,%d,%d\n' % (x,y,int(r*255), int(g*255), int(b*255)));
        	#ft.write('[%d][%d] = %f,%f,%f\n' % (x,y,math.acos(r)*180/math.pi,math.acos(g)*180/math.pi,math.acos(b)*180/math.pi));
        	fwrite1('%c%c%c' % (int(r*255), int(g*255), int(b*255)));
        	    
        f.close();

##############################################################################
##            

def getNumLOSs(filename):
    infile = open(filename, 'r')
    idata = {}
    lines = infile.readlines()#.strip()
    infile.close()
    headline = 0
    ln = lines[headline];
    arr = ln.split();
    return abs(int(arr[2]));
    
    
def fixHeaders(instr):
    """
    Modifies the column header string to be compatible with numpy column lookup.

    Args:
        | list columns (string): column name.


    Returns:
        | list columns (string): fixed column name.

    Raises:
        | No exception is raised.
    """

    intab = u"+-[]@"
    outtab = u"pmbba"

    if isinstance(instr, unicode):
        translate_table = dict((ord(char), unicode(outtab)) for char in intab)
    else:
        assert isinstance(instr, str)
        #print("instr :" + instr);
        translate_table = maketrans(intab, outtab)
        #print(instr.translate(translate_table))
    return instr.translate(translate_table)


##############################################################################
##
def fixHeadersList(headcol):
    """
    Modifies the column headers to be compatible with numpy column lookup.

    Args:
        | list columns ([string]): list of column names.


    Returns:
        | list columns ([string]): fixed list of column names.

    Raises:
        | No exception is raised.
    """

    headcol = [fixHeaders(str) for str in headcol]
    return headcol

##############################################################################
##
def loadtape7(filename, colspec = [], delimiter=None ):

    """
    Read the Modtran tape7 file. This function was tested with Modtran5 files.

    Args:
        | filename (string): name of the input ASCII flatfile.
        | colspec ([string]): list of column names required in the output the spectral transmittance data.

    Returns:
        | numpy.array: an array with the selected columns. Col[0] is the wavenumber.

    Raises:
        | No exception is raised.



    This function reads in the tape7 file from MODerate spectral resolution
    atmospheric TRANsmission (MODTRAN) code, that is used to model the
    propagation of the electromagnetic radiation through the atmosphere. tape7
    is a primary file that contains all the spectral results of the MODTRAN
    run. The header information in the tape7 file contains portions of the
    tape5 information that will be deleted. The header section in tape7 is
    followed by a list of spectral points with corresponding transmissions.
    Each column has a different component of the transmission. For more
    detail, see the modtran documentation.

    The user selects the appropriate columns by listing the column names, as
    listed below.

    The format of the tape7 file changes between different IEMSCT values. For
    the most part the differences are hidden in the details, the user does not
    have to take concern.  The various column headers available are as follows:

    IEMSCT = 0 has two column name lines.  In order to select the column, you
    must concatenate the two column headers with an underscore in between. All
    columns are available with the following column names: ['FREQ_CM-1',
    'COMBIN_TRANS', 'H2O_TRANS', 'UMIX_TRANS', 'O3_TRANS', 'TRACE_TRANS',
    'N2_CONT', 'H2O_CONT', 'MOLEC_SCAT', 'AER+CLD_TRANS', 'HNO3_TRANS',
    'AER+CLD_abTRNS', '-LOG_COMBIN', 'CO2_TRANS', 'CO_TRANS', 'CH4_TRANS',
    'N2O_TRANS', 'O2_TRANS', 'NH3_TRANS', 'NO_TRANS', 'NO2_TRANS',
    'SO2_TRANS', 'CLOUD_TRANS', 'CFC11_TRANS', 'CFC12_TRANS', 'CFC13_TRANS',
    'CFC14_TRANS', 'CFC22_TRANS', 'CFC113_TRANS', 'CFC114_TRANS',
    'CFC115_TRANS', 'CLONO2_TRANS', 'HNO4_TRANS', 'CHCL2F_TRANS',
    'CCL4_TRANS', 'N2O5_TRANS']

    IEMSCT = 1 has single line column headers. A number of columns has
    headers, but with no column numeric data.  In the following list the
    columns with header names ** are empty and hence not available: ['FREQ',
    'TOT_TRANS', 'PTH_THRML', 'THRML_SCT', 'SURF_EMIS', *SOL_SCAT*,
    *SING_SCAT*, 'GRND_RFLT', *DRCT_RFLT*, 'TOTAL_RAD', *REF_SOL*, *SOL@OBS*,
    'DEPTH', 'DIR_EM', *TOA_SUN*, 'BBODY_T[K]']. Hence, these columns do not
    have valid data: ['SOL_SCAT', 'SING_SCAT', 'DRCT_RFLT', 'REF_SOL',
    'SOL@OBS', 'TOA_SUN']

    IEMSCT = 2 has single line column headers. All the columns are available:
    ['FREQ', 'TOT_TRANS', 'PTH_THRML', 'THRML_SCT', 'SURF_EMIS', 'SOL_SCAT',
    'SING_SCAT', 'GRND_RFLT', 'DRCT_RFLT', 'TOTAL_RAD', 'REF_SOL', 'SOL@OBS',
    'DEPTH', 'DIR_EM', 'TOA_SUN', 'BBODY_T[K]']

    IEMSCT = 3 has single line column headers.  One of these seems to be two
    words, which, in this code must be concatenated with an underscore. There
    is also  additional column (assumed to be depth in this code).  The
    columns available are ['FREQ', 'TRANS', 'SOL_TR', 'SOLAR', 'DEPTH']

    """

    infile = open(filename, 'r')
    idata = {}
    lines = infile.readlines()#.strip()
    infile.close()

    #determine values for MODEL, ITYPE, IEMSCT, IMULT from card 1
    #tape5 input format (presumably also tape7, line 1 format?)
    #format Lowtran7  (13I5, F8.3, F7.0) = (MODEL, ITYPE, IEMSCT, IMULT)
    #format Modtran 4 (2A1, I3, 12I5, F8.3, F7.0) = (MODTRN, SPEED, MODEL, ITYPE, IEMSCT, IMULT)
    #format Modtran 5 (3A1, I2, 12I5, F8.0, A7) = (MODTRN, SPEED, BINARY, MODEL, ITYPE, IEMSCT, IMULT)
    #MODEL = int(lines[0][4])
    IEMSCT = int(lines[0][14])
    #IMULT = int(lines[0][19])
    #print('filename={0}, IEMSCT={1}'.format(filename,IEMSCT))

    #skip the first few rows that contains tape5 information and leave the
    #header for the different components of transimissions.
    #find the end of the header.
    headline = 0
    while lines[headline].find('FREQ') < 0:
        headline = headline + 1

    #some files has only a single text column head, while others have two
    # find out what the case is for this file and concatenate if necessary
    colHead1st = lines[headline].split()
    colHead2nd = lines[headline+1].split()
    if colHead2nd[0].find('CM') >= 0:
        colHead = [h1+'_'+h2 for (h1,h2) in zip(colHead1st,colHead2nd)]
        deltaHead = 1
    else:
        colHead = colHead1st
        deltaHead = 0

    #different IEMSCT values have different column formats
    # some cols have headers and some are empty.
    # for IEMSCT of 0 and 2 the column headers are correct and should work as is.
    #for IEMSCT of 1 the following columns are empty and must be deleted from the header

    if IEMSCT == 1:
        removeIEMSCT1 = ['SOL_SCAT', 'SING_SCAT', 'DRCT_RFLT', 'REF_SOL', 'SOL@OBS', 'TOA_SUN']
        colHead = [x for x in colHead if x not in removeIEMSCT1]

    if IEMSCT == 3:
        colHead = ['FREQ', 'TRANS', 'SOL_TR', 'SOLAR', 'DEPTH']

    # build a new data set with appropriate column header and numeric data
    #change all - and +  to alpha to enable table lookup
    colHead = fixHeadersList(colHead)

    s = ' '.join(colHead) + '\n'
    # now append the numeric data, ignore the original header and last row in the file
    s = s + ''.join(lines[headline+1+deltaHead:-1])

    #read the string in from a StringIO in-memory file
    lines = np.ndfromtxt(StringIO.StringIO(s), dtype=None,  names=True)

    #extract the wavenumber col as the first column in the new table
    coldata= lines[fixHeaders(colspec[0])].reshape(-1, 1)
    # then append the other required columns
    for colname in colspec[1:]:
        coldata = np.hstack((coldata, lines[fixHeaders(colname)].reshape(-1, 1)))

    return coldata

 
	
###############################################################################
#
def loadtape7_mod53_ext(filename, colspec = [], first_col = "", delimiter=None ):

    filename_tp5=filename[:filename.find(".")]+".tp5";
    #numLOSs = getNumLOSs(filename_tp5);
    #print("numLOSs: " + str(numLOSs));
    #numWavelen = 74; #152; # number of Wavelength    CARD 4 -> ((700 - 400) / 2) + 2
    
    #(680,550,440) ONLY interested in these wavelengths
    d = CardDeck(filename_tp5,True,10,8,10,10);
    d.readtape5();
    arr = d.card4_arr();
    d.WavelenToInd(arr, 440, 551, 659);
    #d.WavelenToInd(arr, 440, 551, 602);
    numLOSs = d._numLOSs;
    numWavelen = 74; #extract this value from tape5 file
    
    infile = open(filename, 'r')
    idata = {}
    lines = infile.readlines()#.strip()
    infile.close()

    #determine values for MODEL, ITYPE, IEMSCT, IMULT from card 1
    #tape5 input format (presumably also tape7, line 1 format?)
    #format Lowtran7  (13I5, F8.3, F7.0) = (MODEL, ITYPE, IEMSCT, IMULT)
    #format Modtran 4 (2A1, I3, 12I5, F8.3, F7.0) = (MODTRN, SPEED, MODEL, ITYPE, IEMSCT, IMULT)
    #format Modtran 5 (3A1, I2, 12I5, F8.0, A7) = (MODTRN, SPEED, BINARY, MODEL, ITYPE, IEMSCT, IMULT)
    #MODEL = int(lines[0][4])
    ITYPE = int(lines[0][9])
    print("ITYPE : "+str(ITYPE))
    IEMSCT = int(lines[0][14])
    IMULT = int(lines[0][19])
    print("ITYPE : "+str(ITYPE) + ", IMULT : " + str(IMULT) + ", IEMSCT : "+str(IEMSCT))
    print('filename={0}, IEMSCT={1}'.format(filename,IEMSCT))

    #skip the first few rows that contains tape5 information and leave the
    #header for the different components of transimissions.
    #find the end of the header.
    headline = 0
   
    print(lines[headline]);
    #print("first_col: " + str(first_col));
    #while lines[headline].find(first_col) < 0:
    #    headline = headline + 1
    print("LINES: " + lines[headline])
    while lines[headline].find(colspec[0]) < 0:
    	#print("LINES["+str(headline)+"]: " + lines[headline])
        headline = headline + 1

    #some files has only a single text column head, while others have two
    # find out what the case is for this file and concatenate if necessary
    colHead1st = lines[headline].split()
    colHead2nd = lines[headline+1].split()
    #print(colHead2nd)
    #print(colHead1st)
    if colHead2nd[0].find('CM') >= 0:
        colHead = [h1+'_'+h2 for (h1,h2) in zip(colHead1st,colHead2nd)]
        deltaHead = 1
    else:
        colHead = colHead1st
        deltaHead = 0

    #different IEMSCT values have different column formats
    # some cols have headers and some are empty.
    # for IEMSCT of 0 and 2 the column headers are correct and should work as is.
    #for IEMSCT of 1 the following columns are empty and must be deleted from the header

    print("IEMSCT: " + str(IEMSCT))
    if IEMSCT == 1:
        removeIEMSCT1 = ['SOL_SCAT', 'SING_SCAT', 'DRCT_RFLT', 'REF_SOL', 'SOL@OBS', 'TOA_SUN']
        colHead = [x for x in colHead if x not in removeIEMSCT1]

    if IEMSCT == 3:
        colHead = ['FREQ', 'TRANS', 'SOL_TR', 'SOLAR', 'DEPTH']

    # build a new data set with appropriate column header and numeric data
    #change all - and +  to alpha to enable table lookup
    colHead = fixHeadersList(colHead)

    startpos = 0;
    s_ext = 'start';
    coldata = ''
    
    
    #print(str(colHead));
    final_result = []; #np.ndarray( [] );
    arr_val = [];
    while str(s_ext) <> '':
   	#print("startpos: " + str(startpos) + ", headline: " + str(headline));
	#print(lines[headline+1+deltaHead+startpos:headline+1+deltaHead+startpos+numWavelen])
	#s = lines[headline+1+deltaHead+startpos:headline+1+deltaHead+startpos+numWavelen]
	s = ' '.join(colHead) + '\n'
	# now append the numeric data, ignore the original header and last row in the file
	s_ext = ''.join(lines[headline+1+deltaHead+startpos:headline+1+deltaHead+startpos+numWavelen])
	#print(str(headline+1+deltaHead+startpos) + ", " + str(headline+1+deltaHead+startpos+numWavelen));
	#print(lines[(headline+1+deltaHead+startpos):(headline+1+deltaHead+startpos+numWavelen)])
	#print("S_EXT: " + str(s_ext));
	if str(s_ext) == '':
	    break;
	#print("TEST!!!");
	#s = s + s_ext;
	#s = s + ''.join(lines[headline+1+deltaHead+startpos:headline+1+deltaHead+startpos+numWavelen])
	s = s + ''.join(lines[headline+1+deltaHead+startpos:headline+1+deltaHead+startpos+numWavelen])
	data = re.split(r'            ', s);
	s = " 0.000 ".join(str(x).strip() for x in data);
        #print(data)
	
	#print("S-----> " + s)
	#print("S_EXT-----> " + s_ext)
	#read the string in from a StringIO in-memory file
	#np.ndfromtxt(StringIO.StringIO(s), dtype=None,  names=True)
	lines1 = np.ndfromtxt(StringIO.StringIO(s), dtype=None,  names=True)
	
	#extract the wavenumber col as the first column in the new table
	final_result += lines1[first_col].tolist();
	#np.concatenate(final_result, lines1["WAVLEN_NM"]);
	ind_r = d._wv_id[0]; #findInd(lines1["WAVLEN_NM"], 440);
	ind_g = d._wv_id[1];
	ind_b = d._wv_id[2];
	#print("Wavelength inds: " + str(ind_r)+","+str(ind_g)+","+str(ind_b))
	#for ix in range(len(lines1["WAVLEN_NM"])):print(lines1["WAVLEN_NM"][ix])
	#print(lines1[first_col][ind_r]);
	#print(lines1[first_col][ind_g]);
	#print(lines1[first_col][ind_b]);
	val = [lines1[first_col][ind_r],lines1[first_col][ind_g],lines1[first_col][ind_b]]
	arr_val.append(val);
	coldata= lines1[fixHeaders(colspec[0])].reshape(-1, 1)
	## then append the other required columns
	#print(colspec[1:])
    	startpos = startpos + numWavelen + 1;
    	
    #for colname in colspec[1:]:
    #	coldata = np.hstack((coldata, lines1[fixHeaders(colname)].reshape(-1, 1)))
    #print(str(final_result));
    #lines1 = np.ndfromtxt(StringIO.StringIO(s), dtype=None,  names=True)
    #print(lines1["WAVLEN_NM"])
    #return coldata
    return arr_val;



#################################################################
##
def loadtape5(filename, colspec = [], first_col = "", delimiter=None ):

    numWavelen = 152; # number of Wavelength    CARD 4 -> ((700 - 400) / 2) + 2
    infile = open(filename, 'r')
    idata = {}
    lines = infile.readlines()#.strip()
    infile.close()

    #determine values for MODEL, ITYPE, IEMSCT, IMULT from card 1
    #tape5 input format (presumably also tape7, line 1 format?)
    #format Lowtran7  (13I5, F8.3, F7.0) = (MODEL, ITYPE, IEMSCT, IMULT)
    #format Modtran 4 (2A1, I3, 12I5, F8.3, F7.0) = (MODTRN, SPEED, MODEL, ITYPE, IEMSCT, IMULT)
    #format Modtran 5 (3A1, I2, 12I5, F8.0, A7) = (MODTRN, SPEED, BINARY, MODEL, ITYPE, IEMSCT, IMULT)
    #MODEL = int(lines[0][4])
    ITYPE = int(lines[0][9])
    print("ITYPE : "+str(ITYPE))
    IEMSCT = int(lines[0][14])
    IMULT = int(lines[0][19])
    print("ITYPE : "+str(ITYPE) + ", IMULT : " + str(IMULT) + ", IEMSCT : "+str(IEMSCT))
    print('filename={0}, IEMSCT={1}'.format(filename,IEMSCT))

    #skip the first few rows that contains tape5 information and leave the
    #header for the different components of transimissions.
    #find the end of the header.
    headline = 0
   
    print(lines[headline]);
    print(first_col);
    while lines[headline].find(first_col) < 0:
        headline = headline + 1
    #print(lines[headline])
    #while lines[headline].find('FREQ') < 0:
    #    headline = headline + 1

    #some files has only a single text column head, while others have two
    # find out what the case is for this file and concatenate if necessary
    colHead1st = lines[headline].split()
    colHead2nd = lines[headline+1].split()
    #print(colHead2nd)
    #print(colHead1st)
    if colHead2nd[0].find('CM') >= 0:
        colHead = [h1+'_'+h2 for (h1,h2) in zip(colHead1st,colHead2nd)]
        deltaHead = 1
    else:
        colHead = colHead1st
        deltaHead = 0

    #different IEMSCT values have different column formats
    # some cols have headers and some are empty.
    # for IEMSCT of 0 and 2 the column headers are correct and should work as is.
    #for IEMSCT of 1 the following columns are empty and must be deleted from the header

    print("IEMSCT: " + str(IEMSCT))
    if IEMSCT == 1:
        removeIEMSCT1 = ['SOL_SCAT', 'SING_SCAT', 'DRCT_RFLT', 'REF_SOL', 'SOL@OBS', 'TOA_SUN']
        colHead = [x for x in colHead if x not in removeIEMSCT1]

    if IEMSCT == 3:
        colHead = ['FREQ', 'TRANS', 'SOL_TR', 'SOLAR', 'DEPTH']

    # build a new data set with appropriate column header and numeric data
    #change all - and +  to alpha to enable table lookup
    colHead = fixHeadersList(colHead)

    startpos = 0;
    s_ext = 'start';
    coldata = ''
    
    
    #print(str(colHead));
    final_result = []; #np.ndarray( [] );
    while str(s_ext) <> '':
   	print("startpos: " + str(startpos) + ", headline: " + str(headline));
	#print(lines[headline+1+deltaHead+startpos:headline+1+deltaHead+startpos+numWavelen])
	#s = lines[headline+1+deltaHead+startpos:headline+1+deltaHead+startpos+numWavelen]
	s = ' '.join(colHead) + '\n'
	# now append the numeric data, ignore the original header and last row in the file
	s_ext = ''.join(lines[headline+1+deltaHead+startpos:headline+1+deltaHead+startpos+numWavelen])
	#print(str(headline+1+deltaHead+startpos) + ", " + str(headline+1+deltaHead+startpos+numWavelen));
	#print(lines[(headline+1+deltaHead+startpos):(headline+1+deltaHead+startpos+numWavelen)])
	#print("S_EXT: " + str(s_ext));
	if str(s_ext) == '':
	    break;
	#print("TEST!!!");
	#s = s + s_ext;
	#s = s + ''.join(lines[headline+1+deltaHead+startpos:headline+1+deltaHead+startpos+numWavelen])
	s = s + ''.join(lines[headline+1+deltaHead+startpos:headline+1+deltaHead+startpos+numWavelen-1])
	data = re.split(r'            ', s);
	s = " 0.000 ".join(str(x).strip() for x in data);
        #print(data)
	
	#print("S-----> " + s[:300])
	#read the string in from a StringIO in-memory file
	#np.ndfromtxt(StringIO.StringIO(s), dtype=None,  names=True)
	lines1 = np.ndfromtxt(StringIO.StringIO(s), dtype=None,  names=True)
	
	#extract the wavenumber col as the first column in the new table
	final_result += lines1[first_col].tolist();
	#np.concatenate(final_result, lines1["WAVLEN_NM"]);
	#print(lines1["WAVLEN_NM"])
	#coldata= lines1[fixHeaders(colspec[0])].reshape(-1, 1)
	## then append the other required columns
	##print(colspec[1:])
    	startpos = startpos + numWavelen;
    	
    #for colname in colspec[1:]:
	#coldata = np.hstack((coldata, lines1[fixHeaders(colname)].reshape(-1, 1)))
    #print(str(final_result));
    #lines1 = np.ndfromtxt(StringIO.StringIO(s), dtype=None,  names=True)
    #print(lines1["WAVLEN_NM"])
    
    return coldata

#################################################################
##
def savetape7data(filename,tape7):
    """
    Save the numpy array to a text file. Numeric precision is '%.6e'.

    Args:
        | filename (string): name of the output ASCII flatfile.
        | tape7 (numpy.array): an array with the selected columns. Col[0] is the wavenumber.


    Returns:
        | No return.

    Raises:
        | No exception is raised.
    """
    np.savetxt(filename, tape7,fmt='%.6e')



################################################################
##

def driver_gentape5(bTest = False):
    res_mu_s = 5;	#6; #6; #2;    #8
    res_r =  8;		#8; #10;
    res_nu = 6;		#5; #5; #3;    #8
    res_mu = 5;	#7; #4; #4;    #10
    
    dirname = "data_%dr_%dmu_%dmus_%dnu" % (res_r, res_mu, res_mu_s, res_nu)
    if not os.path.exists(dirname):
        os.makedirs(dirname)
    filename = dirname + "/list.txt"
    f = open(filename, 'w')
    for r in range(0,res_r):
    	for y in range(0,res_mu_s):
    	    #d = CardDeck("data/4ucdavis.tp5",True,res_mu_s,8,32,res_r)
    	    #d = CardDeck("",True,res_mu_s,8,10,res_r)
    	    d = CardDeck("",True,res_mu_s,res_nu,res_mu,res_r)
	    fname = dirname + "/out_tape5_"
	    fname = fname + str(r) + "_" +  str(y) + ".tp5"
	    print("FILENAME: " + fname);
	    f.write('%s\n' % fname)
	    d.writetape5(fname, y, r);
    f.close();
    
import os.path	    
def driver_readtape7():
    res_mu_s = 5;	#6; 	#6; #2  #8;
    res_r = 8;		#8; 		#8; #10;
    res_nu = 6;		#5;		#5; #3;   #8;
    res_mu = 5;	#7;		#4; #4;   #10;
    output = [];
    d = CardDeck("",True,res_mu_s,res_nu,res_mu,res_r)
    for r in range(0,res_r):
    	output.append([]);
    	for y in range(0,res_mu_s):
    	    output[r].append([]);
    	    #d = CardDeck("data/4ucdavis.tp5",True,res_mu_s,8,32,res_r)
    	    #d = CardDeck("",True,res_mu_s,8,10,res_r)
	    #fname = "data_7sc_030613_1/out_tape5_" + str(r) + "_" +  str(y) + ".7sc";
	    #fname = "C:/jeffs/mod53ucdavis/mod53ucdavis/TEST1/out_tape5_" + str(r) + "_" +  str(y) + ".7sc";
	    #fname = "data_test/out_tape5_" + str(r) + "_" +  str(y) + ".7sc";
	    fname = "C:/jeffs/mod53ucdavis/mod53ucdavis/"
	    fname = fname + "data_%dr_%dmu_%dmus_%dnu/out_tape5_" % (res_r, res_mu, res_mu_s, res_nu)
	    fname = fname + str(r) + "_" +  str(y) + ".7sc"
	    print(fname);
	    if(os.path.isfile(fname)):
	    	#tape7 = loadtape7_mod53_ext(fname, ['TOTAL_RAD'], 'TOTAL_RAD' );
	    	tape7 = loadtape7_mod53_ext(fname, ['SING_SCAT'], 'SING_SCAT' );
	    	output[r][y].append(tape7);
	    	#print(str(tape7));
	    	#print("_______________________________");
	    else:
	        output[r][y].append([])
	
	d.writeppm('P6', output[r], res_mu_s, res_nu, res_mu, r);
	#writeppm('P6', output[r], res_mu_s, res_nu, res_mu, r);
    #print(str(output));
    
    
################################################################
##

if __name__ == '__init__':
    pass

if __name__ == '__main__':

    import math
    import sys

    #import pyradi.ryplot as ryplot

    figtype = ".png"  # eps, jpg, png
    #figtype = ".eps"  # eps, jpg, png

    ## ----------------------- wavelength------------------------------------------
    #create the wavelength scale to be used in all spectral calculations,
    # wavelength is reshaped to a 2-D  (N,1) column vector
    wavelength=np.linspace(0.38, 0.72, 350).reshape(-1, 1)

    #tape7= loadtape7("data/CD2c3_USS_test.tp7", ['FREQ', 'TOT_TRANS', 'SING_SCAT'] )
    #savetape7data('CD2c3_USS_test.txt',tape7)
    tape7= loadtape7_mod53_ext("data/4ucdavis.tp7", ['FREQ'] )
    savetape7data('4ucdavis.txt',tape7)
    tape7= loadtape7("data/tape7-01", ['FREQ_CM-1', 'COMBIN_TRANS', 'MOLEC_SCAT', 'AER+CLD_TRANS', 'AER+CLD_abTRNS'] )
    savetape7data('tape7-01a.txt',tape7)
    tape7= loadtape7("data/tape7-01", ['FREQ_CM-1', 'COMBIN_TRANS', 'H2O_TRANS', 'UMIX_TRANS', 'O3_TRANS', 'TRACE_TRANS', 'N2_CONT', 'H2O_CONT', 'MOLEC_SCAT', 'AER+CLD_TRANS', 'HNO3_TRANS', 'AER+CLD_abTRNS', '-LOG_COMBIN', 'CO2_TRANS', 'CO_TRANS', 'CH4_TRANS', 'N2O_TRANS', 'O2_TRANS', 'NH3_TRANS', 'NO_TRANS', 'NO2_TRANS', 'SO2_TRANS', 'CLOUD_TRANS', 'CFC11_TRANS', 'CFC12_TRANS', 'CFC13_TRANS', 'CFC14_TRANS', 'CFC22_TRANS', 'CFC113_TRANS', 'CFC114_TRANS', 'CFC115_TRANS', 'CLONO2_TRANS', 'HNO4_TRANS', 'CHCL2F_TRANS', 'CCL4_TRANS', 'N2O5_TRANS'] )
    savetape7data('tape7-01.txt',tape7)
    tape7= loadtape7("data/tape7-02", ['FREQ', 'TOT_TRANS', 'PTH_THRML', 'THRML_SCT', 'SURF_EMIS', 'GRND_RFLT', 'TOTAL_RAD', 'DEPTH', 'DIR_EM', 'BBODY_T[K]'] )
    savetape7data('tape7-02.txt',tape7)
    tape7= loadtape7("data/tape7-03", ['FREQ', 'TOT_TRANS', 'PTH_THRML', 'THRML_SCT', 'SURF_EMIS', 'SOL_SCAT', 'SING_SCAT', 'GRND_RFLT', 'DRCT_RFLT', 'TOTAL_RAD', 'REF_SOL', 'SOL@OBS', 'DEPTH', 'DIR_EM', 'TOA_SUN', 'BBODY_T[K]'] )
    savetape7data('tape7-03.txt',tape7)
    tape7= loadtape7("data/tape7-04", ['FREQ', 'TRANS', 'SOL_TR', 'SOLAR', 'DEPTH'] )
    savetape7data('tape7-04.txt',tape7)

    colSelect =  ['FREQ_CM-1', 'COMBIN_TRANS', 'H2O_TRANS', 'UMIX_TRANS', \
          'O3_TRANS', 'H2O_CONT', 'MOLEC_SCAT', 'AER+CLD_TRANS']
    tape7= loadtape7("data/tape7VISNIR5kmTrop23Vis", colSelect )
    wavelen = ryutils.convertSpectralDomain(tape7[:,0],  type='nl')
    mT = ryplot.Plotter(1, 1, 1,"Modtran Tropical, 23 km Visibility (Rural)"\
                       + ", 5 km Path Length",figsize=(12,6))
    mT.plot(1, wavelen, tape7[:,1:], "","Wavelength [$\mu$m]", "Transmittance",
           label=colSelect[1:],legendAlpha=0.5, pltaxis=[0.4,1, 0, 1],
           maxNX=10, maxNY=4, powerLimits = [-4,  4, -5, 5])
    mT.saveFig('ModtranPlot.png')
    #mT.saveFig('ModtranPlot.eps')

    # this example plots the individual trnansmittance components
    wavelength=np.linspace(0.2, 15, 500).reshape(-1, 1)
    colSelect =  ['FREQ_CM-1', 'COMBIN_TRANS', 'MOLEC_SCAT', 'CO2_TRANS', 'H2O_TRANS', 'H2O_CONT', 'CH4_TRANS',\
       'O3_TRANS', 'O2_TRANS', 'N2O_TRANS', 'AER+CLD_TRANS', 'SO2_TRANS']
    tape7= loadtape7("data/horizon5kmtropical.fl7", colSelect )
    wavelen = ryutils.convertSpectralDomain(tape7[:,0],  type='nl')
    mT = ryplot.Plotter(1, 9, 1,"Modtran Tropical, 23 km Visibility (Rural)"\
                       + ", 5 km Path Length",figsize=(6,12))
    mT.semilogX(1, wavelen, tape7[:,1], '','', '',
           label=colSelect[1:],legendAlpha=0.5, pltaxis=[0.2,15, 0, 1],
           maxNX=10, maxNY=4, powerLimits = [-4,  4, -5, 5])
    mT.semilogX(2, wavelen, tape7[:,2], '','', '',
           label=colSelect[2:],legendAlpha=0.5, pltaxis=[0.2,15, 0, 1],
           maxNX=10, maxNY=4, powerLimits = [-4,  4, -5, 5])
    mT.semilogX(3, wavelen, tape7[:,10], '','', '',
           label=colSelect[10:],legendAlpha=0.5, pltaxis=[0.2,15, 0, 1],
           maxNX=10, maxNY=4, powerLimits = [-4,  4, -5, 5])
    mT.semilogX(4, wavelen, tape7[:,4] , '','', '',
           label=colSelect[4:],legendAlpha=0.5, pltaxis=[0.2,15, 0, 1],
           maxNX=10, maxNY=4, powerLimits = [-4,  4, -5, 5])

    mT.semilogX(5, wavelen, tape7[:,5] , '','', '',
           label=colSelect[5:],legendAlpha=0.5, pltaxis=[0.2,15, 0, 1],
           maxNX=10, maxNY=4, powerLimits = [-4,  4, -5, 5])

    mT.semilogX(6, wavelen, tape7[:,3]  , '','', '',
           label=colSelect[3:],legendAlpha=0.5, pltaxis=[0.2,15, 0, 1],
           maxNX=10, maxNY=4, powerLimits = [-4,  4, -5, 5])
    mT.semilogX(7, wavelen, tape7[:,6]  , '','', '',
           label=colSelect[6:],legendAlpha=0.5, pltaxis=[0.2,15, 0, 1],
           maxNX=10, maxNY=4, powerLimits = [-4,  4, -5, 5])
    mT.semilogX(8, wavelen, tape7[:,7] * tape7[:,8] , '','', '',
           label=colSelect[7:],legendAlpha=0.5, pltaxis=[0.2,15, 0, 1],
           maxNX=10, maxNY=4, powerLimits = [-4,  4, -5, 5])
    mT.semilogX(9, wavelen, tape7[:,9]  , '','', '',
           label=colSelect[9:],legendAlpha=0.5, pltaxis=[0.2,15, 0, 1],
           maxNX=10, maxNY=4, powerLimits = [-4,  4, -5, 5])


    mT.saveFig('ModtranSpec.png')
    mT.saveFig('ModtranSpec.eps')


