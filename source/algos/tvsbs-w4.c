/*
 * SMART: string matching algorithms research tool.
 * Copyright (C) 2012  Simone Faro and Thierry Lecroq
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 * 
 * contact the authors at: faro@dmi.unict.it, thierry.lecroq@univ-rouen.fr
 * download the tool at: http://www.dmi.unict.it/~faro/smart/
 */


#include "include/define.h"
#include "include/main.h"

void TVSBSpreBrBc(unsigned char *x, int m, int brBc[SIGMA][SIGMA]) {
   int a, b, i;
   for (a = 0; a < SIGMA; ++a)
      for (b = 0; b < SIGMA; ++b)
         brBc[a][b] = m + 2;
   for (a = 0; a < SIGMA; ++a)
      brBc[a][x[0]] = m + 1;
   for (i = 0; i < m - 1; ++i)
      brBc[x[i]][x[i + 1]] = m - i;
   for (a = 0; a < SIGMA; ++a)
      brBc[x[m - 1]][a] = 1;
}

 int search(unsigned char *x, int m, unsigned char *y, int n){
   int count,i,s1,s2,s3,s4,j =0;
     int l1,l2,l3,l4;
   int BrBcR[SIGMA][SIGMA], BrBcL[SIGMA][SIGMA];
   unsigned char firstch, lastch;
   unsigned char xr[XSIZE];
   unsigned char c;
     if(n<m+2) return -1;
     if(m<2) return -1;
     
   BEGIN_PREPROCESSING
   for(i=0; i<m; i++) xr[i] = x[m-1-i];
   xr[m]='\0';
   count = 0;
   TVSBSpreBrBc(x, m,  BrBcR);
   TVSBSpreBrBc(xr, m, BrBcL);
   int mm1=m-1, mp1=m+1;
   firstch = x[0];
   lastch = x[m-1];
   END_PREPROCESSING

   BEGIN_SEARCHING
   for(i=0; i<m; i++) y[n+i]=y[n+m+i]=x[i];
   int q = n/2;
   s1 = 0; s2 = q-1; s3 = q; s4 = n-m;
   if(s2>n-m) s2=n-m;
     l1 = s1;
     l3 = s3;
     l2 = s2;
     l4 = s4;

   while(s1<=s2 || s3<=s4){
		if(firstch==y[s1] || firstch==y[s2] || firstch==y[s3] || firstch==y[s4])  {
			if(lastch==y[s1+mm1] || lastch==y[s2+mm1] || lastch==y[s3+mm1] || lastch==y[s4+mm1])  {
				if(s1<l2 && !memcmp(x,y+s1,m)) {
                    l1=s1;
                    count++;
                }
                if(s2>l1 && !memcmp(x,y+s2,m)) {
                    l2=s2;
                    count++;
                }
                if(s3<l4 && !memcmp(x,y+s3,m)) {
                    l3=s3;
                    count++;
                }
                if(s4>l3 && !memcmp(x,y+s4,m)) {
                    l4=s4;
                    count++;
                }
			}
		}
    	s1 += BrBcR[y[s1+m]][y[s1+mp1]];
    	s2 -= BrBcL[y[s2-1]][y[s2-2]];
    	s3 += BrBcR[y[s3+m]][y[s3+mp1]];
    	s4 -= BrBcL[y[s4-1]][y[s4-2]];
    }
   END_SEARCHING
     y[n]='\0';
    return count;
 }
