#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <stdint.h>


inline void initPoly(int *p, int m) {
    int i;
    for(i=0;i<m;i++)
        p[i]=0;
}

void printArr(int *arr, int l){
    int j;
    for(j=0;j<l;j++){
        printf(" %i", arr[j]);
    }
    printf("\n");
}

int mul_mod(int a, int b, int m)
{
   int d = 0, mp2 = m >> 1;
   int i;
   if (a >= m) a %= m;
   if (b >= m) b %= m;
   for (i = 0; i < 64; ++i)
   {
       d = (d > mp2) ? (d << 1) - m : d << 1;
       if (a & 0x8000000000000000)
           d += b;
       if (d > m) d -= m;
       a <<= 1;
   }
   return d;
}

/**
 * copies arr1 into arr2
 */
inline void copyArray(int *arr1, int *arr2, int m){
    int i;
    for(i=0;i<m;i++) arr2[i] = arr1[i];
}

/**
 * copies arr1 with indices idcsArr1 into arr2
 * and copies idcsArr1 into idcsArr2
 */
inline int copyPoly(int *arr1, int *idcsArr1, int lenArr1, 
        int *arr2, int *idcsArr2){
    int i;
    for(i=0;i<lenArr1;i++) arr2[idcsArr1[i]] = arr1[idcsArr1[i]];
    for(i=0;i<lenArr1;i++) idcsArr2[i] = idcsArr1[i];
    return lenArr1;
}


unsigned long long ipow(int base, int exp)
{
    unsigned long long result = 1;
    while (exp)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }

    return result;
}


inline void matmul(int *mat, int *vec, int *ret, int m, int charac){
    int i,j;
    int tmp;
    for(i=0;i<m;i++){
        ret[i] = 0;
        for(j=0;j<m;j++){
            tmp = ( mat[i*m + j]*vec[j] )%charac;
            while(tmp<0) tmp += charac;
            ret[i] += tmp;
        }
    }
}

inline void matmulShort(int *mat, int *vec, int *ret, int m, int charac,
        int *multTable, int *addTable){
    int i,j,tmp,tmp2;
    for(i=0;i<m;i++){
        ret[i] = 0;
        for(j=0;j<m;j++){
            /*tmp = (mat[i*m+j]*vec[j]+charac*charac)%charac;*/
            /*if(tmp != multTable[ mat[i*m+j]*vec[j]] )*/
                /*printf("error matmulShort %i*%i = %i != %i",*/
                        /*mat[i*m+j],vec[j],tmp, multTable[mat[i*m+j]*vec[j]]);*/
            /*tmp2 = ret[i];*/
            ret[i] = addTable[ ret[i] + multTable[ mat[i*m+j]*vec[j] ] ];
            /*tmp = addTable[ tmp2 + multTable[ mat[i*m + j]*vec[j] ] ];*/
            /*if( (ret[i] + charac)%charac != tmp )*/
                /*printf("error matmulShort ret[i]=%i, mat=%i, vec=%i",*/
                        /*ret[i],mat[i*m+j],vec[j]);*/
        }
    }
}

inline int matmulShort_2(int *mat, int *idcsMat, int *lenMat,
        int *vec, int *idcsVec, int lenVec,
        int *ret, int *idcsRet, 
        int m, int charac, int *multTable, int *addTable){
    int i,j,k,pos,tmp,tmp2;
    int curIdxPos = 0;
    int curMatPos;
    bool end;
    /*printf("matmul\n\tmat=\n");*/
    /*for(i=0;i<m;i++){*/
        /*printf("\t\t");printArr(mat+i*m,m);*/
    /*}*/
    /*printf("\tidcsMat=\n");*/
    /*int idcsPosTmp = 0;*/
    /*for(i=0;i<m;i++){*/
        /*printf("\t\t");printArr(idcsMat+idcsPosTmp,lenMat[i]);*/
        /*idcsPosTmp += lenMat[i];*/
    /*}*/
    /*printf("\tvec="); printArr(vec,m);*/
    /*printf("\tidcsVec=");printArr(idcsVec,lenVec);*/

    for(i=0;i<m;i++){
        /*printf("\ti=%i:\n",i);*/
        ret[i] = 0;
        curMatPos = i*m;
        j = 0;
        k = 0;
        end = false;
        while(end == false){
            while(idcsVec[j] > idcsMat[curIdxPos+k]){
                j++;
                if(j == lenVec){
                    end = true;
                    break;
                }
            }
            if(end == true) break;
            while(idcsVec[j] < idcsMat[curIdxPos+k]){
                k++;
                if(k == lenMat[i]){
                    end = true;
                    break;
                }
            }
            if(end == true) break;
            /*printf("\t\tj=%i k=%i\n",j,k);*/
            pos = idcsVec[j]; // == idcsMat[k]
            ret[i] = addTable[ ret[i] 
                + multTable[ mat[curMatPos+pos]*vec[pos] ] ];
            j++;
            k++;
            if(j==lenVec || k==lenMat[i]) end = true;
        }
        curIdxPos += lenMat[i];
    }

    j=0;
    for(i=m-1;i>=0;i--){
        if(ret[i] != 0){
            idcsRet[j] = i;
            j++;
        }
    }
    return j;
}

/* adds 2 polynomials, where p3 is the result
 * MUST have same length */
inline void addPoly(int *p1, int *p2, int *p3, int m, int charac,
        int *multTable, int *addTable) {
    int i;
    for(i=0;i<m;i++)
        p3[i] = addTable[ p1[i]+p2[i] ];
}

/* adds 2 polynomials, where p3 is the result
 * MUST have same length */
inline int addPoly_2(int *p1, int *idcsP1, int lenP1,
        int *p2, int *idcsP2, int lenP2,
        int *p3, int *idcsP3, int charac,
        int *multTable, int *addTable) {
    int i=0,j=0,k=0;
    bool end = false;
    while(end == false){
        while(idcsP1[i] > idcsP2[j]){
            p3[idcsP1[i]] = p1[idcsP1[i]];
            idcsP3[k] = idcsP1[i];
            i++; k++;
        }
        while(idcsP2[j] > idcsP1[i]){
            p3[idcsP2[j]] = p1[idcsP2[j]];
            idcsP3[k] = idcsP2[i];
            j++; k++;
        }
        //idcsP1[i] == idcsP2[j]
        p3[idcsP1[i]] = addTable[ p1[idcsP1[i]] + p2[idcsP1[i]] ]; 
        idcsP3[k] = idcsP1[i];
        k++; i++; j++;
    }
    return k;
}


/* p1 - p2 , where p3 is the result
 * MUST have same length */
void subtrPoly(int *p1, int *p2, int *p3, int m, int charac) {
    int i;
    for(i=0;i<m;i++)
        p3[i] = p1[i]-p2[i];
}

inline void multiplyPoly(int *p1, int m1, int *p2, int m2, 
        int *p3, int m3, int charac) {
    int i,j;
    int tmp;
    int deg1, deg2;
    
    for(i=m1-1;i>=0;i--){
        if(p1[i] != 0){
            deg1 = i;
            break;
        }
    }
    for(i=m2-1;i>=0;i--){
        if(p2[i] != 0){
            deg2 = i;
            break;
        }
    }

    initPoly(p3,m3);
    for(i=0;i<=deg1;i++){
        for(j=0;j<=deg2;j++){
            tmp = ((p1[i]*p2[j])%charac+charac)%charac;
            p3[i+j] = (( p3[i+j]+tmp )%charac+charac)%charac;
        }
    }
    /*for(i=0;i<m3;i++){*/
        /*p3[i] %= charac;*/
        /*if(p3[i] < 0) p3[i] += charac;*/
    /*}*/
}

inline void multiplyPolyKnownDeg(int *p1, int m1, int *p2, int m2, int deg2,
        int *p3, int m3, int charac) {
    int i,j;
    int tmp;
    int deg1;
    
    for(i=m1-1;i>=0;i--){
        if(p1[i] != 0){
            deg1 = i;
            break;
        }
    }

    initPoly(p3,m3);
    for(i=0;i<=deg1;i++){
        for(j=0;j<=deg2;j++){
            p3[i+j] = ((p3[i+j] + p1[i]*p2[j])%charac+charac)%charac;
        }
    }
}


inline void multiplyPolyShort(int *p1, int m1, int *p2, int m2, 
        int *p3, int m3, int charac, int *multTable, int *addTable) {
    int i,j;
    int deg1, deg2;
    int tmp, tmp2;
    for(i=m1-1;i>=0;i--){
        if(p1[i] != 0){
            deg1 = i;
            break;
        }
    }
    for(i=m2-1;i>=0;i--){
        if(p2[i] != 0){
            deg2 = i;
            break;
        }
    }
    initPoly(p3,m3);
    for(i=0;i<=deg1;i++){
        for(j=0;j<=deg2;j++){
            /*tmp2 = p1[i]*p2[j];*/
            /*if( (tmp2+charac*charac)%charac != multTable[tmp2] )*/
                /*printf("error multiplyPolyShort %i*%i=%i != %i",*/
                        /*p1[i],p2[j],(tmp2+charac*charac)%charac, multTable[tmp2]);*/
            p3[i+j] = addTable[ p3[i+j] + multTable[ p1[i]*p2[j] ] ];
        }
    }
}

inline int multiplyPolyShort_2(int *p1, int *idcs1, int len1,
        int *p2, int *idcs2, int len2,
        int *p3, int *idcs3,
        int charac, int *multTable, int *addTable) {
    int i,j,k,l;

    if(len1==0 || len2==0) return 0;
    if(len1==1 && p1[0]==1){
        copyArray(p2,p3,idcs2[0]+1);
        copyArray(idcs2,idcs3,len2);
        return len2;
    }
    if(len2==1 && p2[0]==1){
        copyArray(p1,p3,idcs1[0]+1);
        copyArray(idcs1,idcs3,len1);
        return len1;
    }

    int maxlen = idcs1[0]+idcs2[0];
    initPoly(p3,maxlen+1);
    for(i=0;i<len1;i++){
        for(j=0;j<len2;j++){
            k = idcs1[i];
            l = idcs2[j];
            p3[k+l] = addTable[ p3[k+l] + multTable[ p1[k]*p2[l] ] ];
        }
    }
    j=0;
    for(i=maxlen;i>=0;i--){
        if(p3[i] != 0){
            idcs3[j] = i;
            j++;
        }
    }
    return j;
}


inline void multiplyPolyShortKnownDeg(int *p1, int m1, int *p2, int m2, int deg2,
        int *p3, int m3, int charac, int *multTable, int *addTable) {
    int i,j;
    int deg1, deg2Tmp;
    for(i=m1-1;i>=0;i--){
        if(p1[i] != 0){
            deg1 = i;
            break;
        }
    }
    /*for(i=m1-1;i>=0;i--){*/
        /*if(p2[i] != 0){*/
            /*deg2Tmp = i;*/
            /*break;*/
        /*}*/
    /*}*/
    initPoly(p3,m3);
    
    if(deg2 == -1) return;
    /*printf("multiplyPolyShortKnownDeg p1=");printArr(p1,m1);*/
    /*printf("                          p2=");printArr(p2,m2);*/
    /*printf("                     deg1=%i deg2=%i\n",deg1,deg2);*/

    for(i=0;i<=deg1;i++){
        for(j=0;j<=deg2;j++){
            /*p3[i+j] += p1[i]*p2[j];*/
            /*printf("              p1[%i]*p2[%i]=%i <-> %i\n",*/
                    /*i,j,p1[i]*p2[j],multTable[p1[i]*p2[j]]);*/
            p3[i+j] = addTable[ p3[i+j] + multTable[ p1[i]*p2[j] ] ];
        }
    }
}



/*
 * calculates a^(-1) mod p
 */
inline int modInv(int a, int p){
    int s,t,r,old_s,old_t,old_r, quo, tmp;
    s = 0; old_s = 1;
    t = 1; old_t = 0;
    r = p; old_r = a;
    while(r != 0){
        quo = old_r / r;
        tmp = r; r = old_r - quo*tmp; old_r = tmp;
        tmp = s; s = old_s - quo*tmp; old_s = tmp;
        tmp = t; t = old_t - quo*tmp; old_t = tmp;
    }
    // bezout coeffs: old_s old_t
    // gcd: old_r
    // quotients for gcd: t s
    return abs(old_s);
}

inline void moduloPoly(int *p1, int m1, int *mod, int m, int charac){
    int deg1=0, degmod=0;
    int i=0,j=0;
    long quo=0;
    //get degrees
    for(i=m1-1;i>=0;i--){
        if( p1[i] != 0){
            deg1 = i;
            break;
        }
    }
    for(i=m-1;i>=0;i--){
        if( mod[i] != 0){
            degmod = i;
            break;
        }
    }
    /*printf("poly = ");printArr(p1,m1);*/
    /*printf("mod = ");printArr(mod,m);*/
    /*printf("degMod=%i, degP1=%i\n", degmod,deg1);*/
    
    /*printArr(p1,m1);*/
    /*int *tmpArr = malloc(m1*sizeof(int));*/

    //make polynomial division
    int degmodInv = modInv(mod[degmod],charac);
    for(i=deg1-degmod; i>=0; i--){
        quo = (p1[i+degmod]*(long)degmodInv)%charac;
        /*printf("i=%i p1[i+degmod]=%i mod[degmod]=%i mod[degmod]^(-1)=%i ",i,p1[i+degmod],mod[degmod],modInv(mod[degmod],charac));*/
        /*printf("quo=%lu\n", quo);*/
        for(j=degmod;j>=0;j--){
            p1[i+j] = (int)((p1[i+j] - mod[j]*quo)%charac);
        }
        /*for(j=0;j<m1;j++){*/
            /*tmpArr[j] = p1[j];*/
            /*if(tmpArr[j] < 0) tmpArr[j] += charac;*/
        /*}*/

    /*printf("=> p1=");printArr(tmpArr,m1);*/
}
    /*for(i=0;i<m1;i++){*/
        /*if(p1[i] < 0) p1[i] += charac;*/
    /*}*/
    /*free(tmpArr);*/
}

inline void moduloMonom(int *p1, int m1, int *mod, int m, int charac){
    int deg1, degmod=m-1;
    int quo, tmp;
    int i,j;
    //get degrees
    for(i=m1-1;i>=0;i--){
        if( p1[i] != 0){
            deg1 = i;
            break;
        }
    }
    //make polynomial division
    for(i=deg1-degmod; i>=0; i--){
        quo = p1[i+degmod]%charac;
        for(j=degmod;j>=0;j--){
            p1[i+j] =  (( p1[i+j] - mod[j]*quo )%charac+charac)%charac;
        }
    }
}

inline void moduloMonomShort(int *p1, int m1, int *mod, int m, int charac,
        int *multTable, int *addTable){
    int deg1=0, degmod=m-1;
    int i=0,j=0;
    int quo=0;
    //get degrees
    for(i=m1-1;i>=0;i--){
        if( p1[i] != 0){
            deg1 = i;
            break;
        }
    }
    /*for(i=m-1;i>=0;i--){*/
        /*if( mod[i] != 0){*/
            /*degmod = i;*/
            /*break;*/
        /*}*/
    /*}*/

    //make polynomial division
    for(i=deg1-degmod; i>=0; i--){
        quo = p1[i+degmod];
        for(j=degmod;j>=0;j--){
            /*p1[i+j] -= mod[j]*quo;*/
            /*p1[i+j] = multTable[ mod[j]*quo ];*/
            p1[i+j] = addTable[ p1[i+j] - multTable[mod[j]*quo] ];
        }
    }
}

inline int moduloMonomShort_2(int *p1, int *idcs1, int len1, 
        int *mod, int *idcsMod, int lenMod, 
        int charac, int *multTable, int *addTable){

    int i,j,k,l,j2;
    int degmod = idcsMod[0];
    int quo;

    if(idcs1[0] < degmod) return len1;

    for(i=idcs1[0]-degmod; i>=0; i--){
        quo = p1[i+degmod];
        if(quo == 0) continue;
        for(j=0;j<lenMod;j++){
            j2 = idcsMod[j];
            p1[i+j2] = addTable[ p1[i+j2] - multTable[mod[j2]*quo] ];
        }
    }

    j=0;
    for(i=degmod-1;i>=0;i--){
        if(p1[i] != 0){
            idcs1[j] = i;
            j++;
        }
    }
    return j;
}


/**
 * Square and multiply
 * x is modified!
 */
inline void powerPolyInt(int *x, int *x_mipo, int *ret, int m, 
        int power, int charac, int *tmp2){
    int i,j;
    initPoly(ret,m);
    ret[0] = 1;
    while(power > 0){
        if(power & 1 == 1){
            multiplyPoly(ret,m, x,m, tmp2,2*m, charac);
            moduloPoly(tmp2,2*m,x_mipo,m+1,charac);
            copyArray(tmp2,ret,m);
        }
        multiplyPoly(x,m, x,m, tmp2,2*m, charac);
        /*printf("powerPolyInt tmp2=");printArr(tmp2,2*m);*/
        moduloPoly(tmp2,2*m,x_mipo,m+1,charac);
        copyArray(tmp2,x,m);
        power >>= 1;
    }
}
/**
 * Square and multiply
 * x is modified!
 */
inline void powerPoly(int *x, int *x_mipo, int *ret, int m, 
        char *power, int powerLen,
        int charac, int *tmp2){
    int i,j;
    initPoly(ret,m);
    ret[0] = 1;
    for(j=powerLen-1;j>=0;j--){
        if(power[j] == 1){
            multiplyPoly(ret,m, x,m, tmp2,2*m, charac);
            moduloMonom(tmp2,2*m,x_mipo,m+1,charac);
            for(i=0;i<m;i++)
                ret[i] = tmp2[i];
        }
        multiplyPoly(x,m, x,m, tmp2,2*m, charac);
        moduloMonom(tmp2,2*m,x_mipo,m+1,charac);
        for(i=0;i<m;i++)
            x[i] = tmp2[i];
    }
}
/**
 * Square and multiply
 * x is modified!
 */
inline void powerPolyShortInt(int *x, int *x_mipo, int *ret, int m, 
        int power, int charac, int *tmp2, 
        int *multTable, int *addTable){
    int i,j;
    initPoly(ret,m);
    ret[0] = 1;
    while(power > 0){
        if(power & 1 == 1){
            multiplyPolyShort(ret,m, x,m, tmp2,2*m, charac,multTable,addTable);
            moduloMonomShort(tmp2,2*m,x_mipo,m+1,charac,multTable,addTable);
            /*for(i=0;i<m;i++)*/
                /*ret[i] = tmp2[i]%charac;*/
            copyArray(tmp2,ret,m);
        }
        multiplyPolyShort(x,m, x,m, tmp2,2*m, charac,multTable,addTable);
        moduloMonomShort(tmp2,2*m,x_mipo,m+1,charac,multTable,addTable);
        /*for(i=0;i<m;i++)*/
            /*x[i] = tmp2[i]%charac;*/
        copyArray(tmp2,x,m);
        power >>= 1;
    }
}

/**
 * Square and multiply
 * x is modified!
 */
inline int powerPolyShortInt_2(int *x, int *idcsX, int lenX,
        int *x_mipo, int *idcsMipo, int lenMipo, 
        int *ret, int *idcsRet, 
        int m, 
        int power, int charac, int *tmp2, 
        int *multTable, int *addTable){
    int i,j;
    initPoly(ret,m);
    ret[0] = 1;
    int lenRet = 1;
    while(power > 0){
        if(power & 1 == 1){
            lenRet = multiplyPolyShort_2(ret,idcsRet,lenRet, 
                    x,idcsX,lenX, 
                    tmp2, idcsRet,
                    charac,multTable,addTable);
            lenRet = moduloMonomShort_2(tmp2, idcsRet,lenRet,
                    x_mipo,idcsMipo,lenMipo,
                    charac,multTable,addTable);
            /*for(i=0;i<m;i++)*/
                /*ret[i] = tmp2[i]%charac;*/
            copyArray(tmp2,ret,idcsRet[0]+1);
        }
        lenX = multiplyPolyShort_2(x,idcsX,lenX, 
                x,idcsX,lenX, 
                tmp2,idcsX, 
                charac,multTable,addTable);
        lenX = moduloMonomShort_2(tmp2,idcsX,lenX,
                x_mipo,idcsMipo,lenMipo,
                charac,multTable,addTable);
        copyArray(tmp2,x,idcsX[0]+1);
        power >>= 1;
    }
    return lenRet;
}

/**
 * Square and multiply in charac
 * mat is powering by charac
 * x is modified!
 */
inline void powerPolyShortCharac(int *x, int *x_mipo, int *ret, int m, 
        int *power, int powerLen, int *matCharac,
        int charac, int *tmp2, int *multTable, int *addTable){
    int i,j,k, lenCurGap = 0, mSize = m*m;
    initPoly(ret,m);
    ret[0] = 1;
    /*printf("powerPolyShortCharac x=");printArr(x,m);*/
    for(j=powerLen-1;j>=0;j--){
        /*printf("j=%i power[j]=%i\n",j,power[j]);*/
        for(k=0; k<power[j]; k++){
            multiplyPolyShort(ret,m, x,m, tmp2,2*m, charac, multTable,addTable);
            moduloMonomShort(tmp2,2*m,x_mipo,m+1,charac,multTable,addTable);
            for(i=0;i<m;i++)
                ret[i] = tmp2[i];
                /*ret[i] = tmp2[i]%charac;*/
        }
        /*printf("\tret=");printArr(ret,m);*/
        
        if(j==0 || power[j-1] == 0){
            lenCurGap++;
            continue;
        }
        /*printf("matmul lenCurGap=%i\n",lenCurGap);*/
        matmulShort(matCharac+lenCurGap*mSize,x,tmp2,m,charac,multTable,addTable);
        for(i=0;i<m;i++)
            x[i] = tmp2[i];
            /*x[i] = tmp2[i]%charac;*/
        lenCurGap = 0;
        /*printf("\t  x=");printArr(x,m);*/
    }
    /*printf("\tret=");printArr(ret,m);*/
}

/**
 * Square and multiply in charac
 * mat is powering by charac
 *
 * x is modified!
 * tmpRet must be 2*m long!
 */
inline int powerPolyShortCharac_2(int *x, int *idcsX, int lenX,
        int *x_mipo, int *idcsMipo, int lenMipo,
        int *ret, int *idcsRet, 
        int m, int *power, int powerLen, 
        int *matCharac, int *idcsMatCharac, int *lenMatCharac,
        int charac, int *tmp2, int *multTable, int *addTable){
    int i,j,k, lenCurGap = 0, mSize = m*m;
    int lenRet;
    int curMatIdx, i2;
    initPoly(ret,m);
    ret[0] = 1;  idcsRet[0] = 0;  lenRet = 1;
    /*printf("-----------------------------------------------------------------\n");*/
    /*printf("powerPoly x=");printArr(x,m);*/
    /*printf("        idcsX=");printArr(idcsX,lenX);*/
    /*printf("power=");printArr(power,powerLen);*/
    
    for(j=powerLen-1;j>=0;j--){
        /*printf("j=%i power[j]=%i\n",j,power[j]);*/
        /*printf("\tret=");printArr(ret,m);*/
        /*printf("\tidcsRet=");printArr(idcsRet,lenRet);*/
        /*printf("\tx=");printArr(x,m);*/
        /*printf("\tidcsX=");printArr(idcsX,lenX);*/

        for(k=0; k<power[j]; k++){
            /*printf("\t\tk=%i\n",k);*/
            lenRet = multiplyPolyShort_2(
                    ret, idcsRet, lenRet,
                    x, idcsX, lenX, 
                    tmp2, idcsRet,
                    charac, multTable,addTable);
            /*printf("\t\tx*ret = tmp2=");printArr(tmp2,2*m);*/
            /*printf("\t\t        idcsTmp2=");printArr(idcsRet,lenRet);*/
            lenRet = moduloMonomShort_2(tmp2, idcsRet, lenRet,
                    x_mipo, idcsMipo, lenMipo, 
                    charac,multTable,addTable);
            for(i=0;i<m;i++)
                ret[i] = tmp2[i];
            /*printf("\t\tret=");printArr(ret,m);*/
            /*printf("\t\tidcsRet=");printArr(idcsRet,lenRet);*/
        }
        /*printf("\tret=");printArr(ret,m);*/
        /*printf("\tidcsRet=");printArr(idcsRet,lenRet);*/
        
        if(j==0 || power[j-1] == 0){
            lenCurGap++;
            continue;
        }
        /*printf("\tmatmul lenCurGap=%i\n",lenCurGap);*/
        curMatIdx = 0;
        for(i=0;i<lenCurGap*m;i++)
            curMatIdx += lenMatCharac[i];
            lenX = matmulShort_2(matCharac+lenCurGap*mSize, idcsMatCharac+curMatIdx,
                lenMatCharac+lenCurGap*m,
                x, idcsX, lenX,
                tmp2,idcsX,
                m,charac,multTable,addTable);
        copyArray(tmp2,x,idcsX[0]+1);
        lenCurGap = 0;
        /*printf("\tx=");printArr(x,m);*/
        /*printf("\tidcsX=");printArr(idcsX,lenX);*/
    }
    /*printf("\tret=");printArr(ret,m);*/
    /*printf("------------------- end = true;----------------------------------------------\n");*/
    return lenRet;
}

///**
// * Square and multiply
// * x is modified!
// */
//inline void powerPolyShort(int *x, int *x_mipo, int *ret, int m, 
//        char *power, int powerLen,
//        int charac, int *tmp2){
//    int i,j;
//    initPoly(ret,m);
//    ret[0] = 1;
//    for(j=powerLen-1;j>=0;j--){
//        if(power[j] == 1){
//            multiplyPolyShort(ret,m, x,m, tmp2,2*m, charac);
//            moduloMonomShort(tmp2,2*m,x_mipo,m+1,charac);
//            for(i=0;i<m;i++)
//                ret[i] = tmp2[i]%charac;
//        }
//        multiplyPolyShort(x,m, x,m, tmp2,2*m, charac);
//        moduloMonomShort(tmp2,2*m,x_mipo,m+1,charac);
//        for(i=0;i<m;i++)
//            x[i] = tmp2[i]%charac;
//    }
//}

inline bool isOne(int *x, int m, int charac){
    int i=0;
    bool allZ = true;
    if(!(x[0] == 1 || x[0] == -charac+1))
        return false;
    for(i=1;i<m;i++){
        if(x[i] != 0){
            allZ = false;
            break;
        }
    }
    return allZ;
}

/**
 * multiplies 2 quadratic m x m matrices 
 */
void multMatrices(int *mat1, int *mat2, int *ret, int m, int charac){
    int i,j,k;
    int tmp;
    for(i=0;i<m;i++){
        for(j=0;j<m;j++){
            ret[i*m + j] = 0;
            for(k=0;k<m;k++){
                tmp = ( mat1[i*m+k]*mat2[k*m+j] )%charac;
                while(tmp<0) tmp += charac;
                tmp = ( ret[i*m+j] + tmp )%charac;
                while(tmp<0) tmp += charac;
                ret[i*m + j] = tmp;
            }
        }
    }
}
void multMatricesShort(int *mat1, int *mat2, int *ret, int m, int charac,
        int *multTable,int *addTable){
    int i,j,k;
    /*long tmp;*/
    for(i=0;i<m;i++){
        for(j=0;j<m;j++){
            ret[i*m + j] = 0;
            for(k=0;k<m;k++){
                ret[i*m+j] = addTable[ ret[i*m+j] + 
                            multTable[ mat1[i*m+k]*mat2[k*m+j] ] ];
                /*tmp = (long)mat1[i*m+k]*(long)mat2[k*m+j];*/
                /*ret[i*m + j] = (int)((ret[i*m + j] + tmp)%charac);*/
            }
        }
    }
}


void genMats(int *mipo, int m, int *mats, int maxPower, int charac, int q){
    int *tmp = malloc(m*sizeof(int));
    int *tmp2 = malloc(2*m*sizeof(int));
    int *x = malloc(m*sizeof(int));

    int i,j,k;
    int mSize = m*m;
    /*[>//put identity matrix at position 0<]*/
    /*for(i=0;i<m;i++)*/
        /*for(j=0;j<m;j++)*/
            /*if(j==i)*/
                /*[>matsInt[0][i][j] = 1;<]*/
                /*mats[0*mSize + i*m + j] = 1;*/
            /*else*/
                /*[>matsInt[0][i][j] = 0;<]*/
                /*mats[0*mSize + i*m + j] = 0;*/

    for(i=0;i<m;i++){
        initPoly(x,m);
        x[i] = 1;
        powerPolyInt(x, mipo, tmp, m, q, charac, tmp2);
        for(j=0;j<m;j++)
            /*matsInt[1][j][i] = tmp[j];*/
            mats[j*m + i] = tmp[j];
    }
    for(i=1;i<=maxPower;i++){
        /*multMatrices(matsInt[1],matsInt[i-1],matsInt[i], m, charac);*/
        multMatrices(mats, mats+(i-1)*mSize, mats+i*mSize, m, 
                charac);
    }

    /*for(i=0;i<=maxPower;i++){*/
        /*printf("m^%i\n",i);*/
        /*for(j=0;j<m;j++)*/
            /*printArr(mats+mSize*i+j*m,m);*/
    /*}*/

    free(x);
    free(tmp2);
    free(tmp);
}



/**
 * Calc order of element
 * x is NOT modified!
 */
inline bool isPrimitive(int *x, int *x_mipo, int m, 
        int *barFactors, int *lenBarFactors, int countBarFactors, 
        int *biggestPrimeFactor, int lenBiggestPrimeFactor,
        int *matCharac, int charac,
        int *tmp_x, int *tmp, int *tmpRet, int *tmpRet2, int *tmp2,
        int *multTable, int *addTable){
    int i,j, curPos=0;
    /*printf("isPrimitive x=");printArr(x,m);*/
    //test first barFactor
    for(j=0;j<m;j++) tmp_x[j] = x[j];
    /*printf("firstFactor=");printArr(barFactors,lenBarFactors[0]);*/
    powerPolyShortCharac(tmp_x, x_mipo, tmp, m, barFactors,
            lenBarFactors[0], matCharac, charac, tmp2,
            multTable,addTable);
    /*printf("x^firstFactor=");printArr(tmp,m);*/
    if( isOne(tmp,m, charac) == true)
        return false;
    curPos += lenBarFactors[0];
    // test further factors which are powers of biggesPrimeFactor
    for(j=0;j<m;j++) tmp_x[j] = x[j];
    /*printf("biggestPrimeFactor=");printArr(biggestPrimeFactor,lenBiggestPrimeFactor);*/
    powerPolyShortCharac(tmp_x, x_mipo, tmpRet, m, biggestPrimeFactor,
            lenBiggestPrimeFactor, matCharac, charac, tmp2,
            multTable,addTable);
    /*printf("x^biggesPrimeFactor=");printArr(tmpRet,m);*/
    for(i=1;i<countBarFactors;i++){
        /*printf("calc y^ ");printArr(barFactors+curPos,lenBarFactors[i]);*/
        for(j=0;j<m;j++) tmp_x[j] = tmpRet[j];
        powerPolyShortCharac(tmp_x, x_mipo, tmp, m, barFactors+curPos,
                lenBarFactors[i], matCharac, charac, tmp2, multTable,addTable);
        /*printf("      =>");printArr(tmp,m);*/
        if(i>1){
            /*printf("   *");printArr(tmpRet2,m);*/
            multiplyPolyShort(tmp,m,tmpRet2,m,tmp2,2*m,charac,multTable,addTable);
            moduloMonomShort(tmp2,2*m,x_mipo,m+1,charac,multTable,addTable);
            copyArray(tmp2,tmpRet2,m);
            /*for(j=0;j<m;j++) tmpRet2[j] %= charac;*/
            /*printf("      =>");printArr(tmpRet2,m);*/
        }else{
            copyArray(tmp,tmpRet2,m);
        }
        if( isOne(tmpRet2,m, charac) == true)
            return false;
        curPos += lenBarFactors[i];
    }
    return true;
}

/**
 * Calc order of element
 *
 * x is NOT modified!
 * idcsTmp, idcsTmp_x, idcsTmpRet2 must be 2*m long!
 */
inline bool isPrimitive_2(int *x, int *idcsX, int lenX,
        int *x_mipo, int *idcsMipo, int lenMipo, 
        int m, 
        int *barFactors, int *lenBarFactors, int countBarFactors, 
        int *biggestPrimeFactor, int lenBiggestPrimeFactor,
        int *matCharac, int *idcsMatCharac, int *lenMatCharac, int charac,
        int *tmp_x, int *idcsTmp_x, int *tmp, int *idcsTmp,
        int *tmpRet, int *idcsTmpRet, 
        int *tmpRet2, int *idcsTmpRet2, int *tmp2,
        int *multTable, int *addTable){
    int i,j, curPos=0;
    int lenTmp, lenTmp_x, lenTmpRet2, lenTmpRet;
    /*printf("isPrimitive x=");printArr(x,m);*/
    /*printf("\tidcsX=");printArr(idcsX,lenX);*/
    //test first barFactor
    copyArray(x,tmp_x,m);
    copyArray(idcsX,idcsTmp_x,lenX);
    /*printf("firstFactor=");printArr(barFactors,lenBarFactors[0]);*/
    lenTmp = powerPolyShortCharac_2(tmp_x, idcsTmp_x, lenX,
            x_mipo, idcsMipo, lenMipo, 
            tmp, idcsTmp,
            m, barFactors,
            lenBarFactors[0], matCharac, idcsMatCharac, lenMatCharac,
            charac, tmp2, multTable,addTable);
    /*printf("x^firstFactor=");printArr(tmp,m);*/
    /*printf("         idcs=");printArr(idcsTmp,lenTmp);*/
    if( lenTmp == 1 && tmp[0] == 1)
        return false;
    curPos += lenBarFactors[0];
    // test further factors which are powers of biggestPrimeFactor
    copyArray(x,tmp_x,idcsX[0]+1);
    copyArray(idcsX,idcsTmp,lenX);
    lenTmpRet = powerPolyShortCharac_2(tmp_x, idcsTmp, lenX,
            x_mipo, idcsMipo, lenMipo,
            tmpRet, idcsTmpRet,
            m, biggestPrimeFactor,
            lenBiggestPrimeFactor, 
            matCharac, idcsMatCharac, lenMatCharac,
            charac, tmp2, multTable,addTable);
    /*printf("x^biggesPrimeFactor=");printArr(tmpRet,m);*/
    /*printf("               idcs=");printArr(idcsTmpRet,lenTmpRet);*/
    for(i=1;i<countBarFactors;i++){
        /*printf("calc y^ ");printArr(barFactors+curPos,lenBarFactors[i]);*/
        //copy x^biggestprimefactor (tmpRet) to tmp_x ----
        copyArray(tmpRet,tmp_x,idcsTmpRet[0]+1);
        copyArray(idcsTmpRet,idcsTmp_x,lenTmpRet);
        lenTmp_x = lenTmpRet;
        //---
        lenTmp = powerPolyShortCharac_2(tmp_x, idcsTmp_x, lenTmp_x,
                x_mipo, idcsMipo, lenMipo,
                tmp, idcsTmp,
                m, barFactors+curPos,
                lenBarFactors[i], 
                matCharac, idcsMatCharac, lenMatCharac,
                charac, tmp2, multTable,addTable);
        /*printf("      =>tmp=");printArr(tmp,m);*/
        /*printf("\tidcsTmp=");printArr(idcsTmp,lenTmp);*/
        if(i>1){
            /*printf("   *");printArr(tmpRet2,m);*/
            lenTmpRet2 = multiplyPolyShort_2(tmp,idcsTmp,lenTmp,
                    tmpRet2,idcsTmpRet2,lenTmpRet2,
                    tmp2, idcsTmpRet2, 
                    charac,multTable,addTable);
            lenTmpRet2 = moduloMonomShort_2(tmp2, idcsTmpRet2, lenTmpRet2,
                    x_mipo, idcsMipo, lenMipo,
                    charac,multTable,addTable);
            copyArray(tmp2,tmpRet2,idcsTmpRet2[0]+1);
            /*printf("      =>tmpRet2");printArr(tmpRet2,m);*/
            /*printf("\tidcsTmpRet2=");printArr(idcsTmpRet2,lenTmpRet2);*/
        }else{
            copyArray(tmp,tmpRet2,idcsTmp[0]+1);
            copyArray(idcsTmp,idcsTmpRet2,lenTmp);
            lenTmpRet2 = lenTmp;
        }
        if( lenTmpRet2 == 1 && tmpRet2[0] == 1)
            return false;
        curPos += lenBarFactors[i];
    }
    return true;
}

inline bool allZero(int *x, int m){
    int i=0;
    bool allZ = true;
    for(i=0;i<m;i++){
        if(x[i] != 0){
            allZ = false;
            break;
        }
    }
    return allZ;
}



/*
 * calculates g(sigma)(x) where g is a polynomial and sigma the frobenius
 * application of frobenius is given by mats, i.e.
 * mats is array of m x m matrices starting with ^q matrix, 
 * x an array of m ints
 * g is an array of glen arrays of length m
 */
inline void applyFrob(int *x, int *x_mipo, 
        int *g, int glen, int *gCoeffDegs,
        int *mats, int frobpower,
        int *ret, int m, int charac, int *tmp, int *tmp2, int *tmp_x,
        int *matmulCache, bool *matmulCacheCalced){
    int mSize = m*m;
    int i,j,k,l;
    int curMatPower = 0;
    initPoly(ret,m);
    /*printf("applyFrobShort glen=%i x=",glen);printArr(x,m);*/
    for(i=0;i<glen;i++){
        copyArray(x,tmp_x,m);
        /*printf("   gi=");printArr(g+i*m,m);*/
        if(allZero(g+i*m,m) == true){
            /*printf("   is zero -> skip\n((E.gen()**j)**(q**i)");*/
            curMatPower++;
            continue;
        }
        if(matmulCacheCalced[i*frobpower] == 1){
            /*printf("   x^%i cached=",i*frobpower);*/
                                    /*printArr(matmulCache+m*(i*frobpower),m);*/
            multiplyPoly(matmulCache+m*(i*frobpower),m,
                    g+i*m, m, tmp2, 2*m, charac);
            copyArray(matmulCache+m*(i*frobpower), tmp_x, m);
            curMatPower = 0;
        }else{
            matmul(mats+mSize*curMatPower, tmp_x, tmp, m, charac);
            curMatPower = 0;
            /*printf("   x^%i=",i*frobpower);printArr(tmp,m);*/
            for(j=0;j<m;j++)
                matmulCache[m*(i*frobpower)+j] = tmp[j];
            copyArray(matmulCache+m*(i*frobpower), tmp_x, m);
            matmulCacheCalced[i*frobpower] = 1;
            multiplyPolyKnownDeg(tmp,m, g+i*m, m, gCoeffDegs[i],
                    tmp2, 2*m, charac);
            /*printf("   gi*x^i=");printArr(tmp2,2*m);*/
            /*multiplyPolyShort(tmp,m, g+i*m, m, */
                    /*tmp2, 2*m, charac);*/
        }
        moduloMonom(tmp2, 2*m, x_mipo, m+1, charac);
        /*printf("   gi*x^i mod=");printArr(tmp2,2*m);*/
        for(j=0;j<m;j++){
            ret[j] += tmp2[j]; 
            while( ret[j]<0 ) ret[j] += charac;
        }
        /*printf("      => ret=");printArr(ret,m);*/
    }
}

/*
 * calculates g(sigma)(x) where g is a polynomial and sigma the frobenius
 * application of frobenius is given by mats, i.e.
 * mats is array of m x m matrices, x an array of m ints
 * g is an array of glen arrays of length m
 */
inline void applyFrobShort(int *x, int *x_mipo, 
        int *g, int glen, int *gCoeffDegs,
        int *mats, int frobpower,
        int *ret, int m, int charac, int *tmp, int *tmp2,
        int *matmulCache, bool *matmulCacheCalced,
        int *multTable, int *addTable){
    int mSize = m*m;
    int i,j,k,l;
    initPoly(ret,m);
    /*printf("applyFrobShort glen=%i x=",glen);printArr(x,m);*/
    for(i=0;i<glen;i++){
        /*printf("   gi=");printArr(g+i*m,m);*/
        if(allZero(g+i*m,m) == true){
            /*printf("   is zero -> skip\n");*/
            continue;
        }
        if(matmulCacheCalced[i*frobpower] == 1){
            /*printf("   x^%i cached=",i*frobpower);*/
                                    /*printArr(matmulCache+m*(i*frobpower),m);*/
            multiplyPolyShort(matmulCache+m*(i*frobpower),m,
                    g+i*m, m, tmp2, 2*m, charac, multTable,addTable);
        }else{
            matmulShort(mats+i*frobpower*mSize, x, tmp, m, charac,
                    multTable,addTable);
            /*printf("   x^%i=",i*frobpower);printArr(tmp,m);*/
            for(j=0;j<m;j++)
                matmulCache[m*(i*frobpower)+j] = tmp[j];
            matmulCacheCalced[i*frobpower] = 1;
            multiplyPolyShortKnownDeg(tmp,m, g+i*m, m, gCoeffDegs[i],
                    tmp2, 2*m, charac,multTable,addTable);
            /*printf("   gi*x^i=");printArr(tmp2,2*m);*/
            /*multiplyPolyShort(tmp,m, g+i*m, m, */
                    /*tmp2, 2*m, charac);*/
        }
        moduloMonomShort(tmp2, 2*m, x_mipo, m+1, charac,multTable,addTable);
        /*printf("   gi*x^i mod=");printArr(tmp2,2*m);*/
        for(j=0;j<m;j++){
            ret[j] = addTable[ ret[j] + tmp2[j] ];
        }
        /*printf("      => ret=");printArr(ret,m);*/
    }
    /*for(i=0;i<m;i++){*/
        /*ret[i] %= charac;*/
    /*}*/
}
/*
 * calculates g(sigma)(x) where g is a polynomial and sigma the frobenius
 * application of frobenius is given by mats, i.e.
 * mats is array of m x m matrices, x an array of m ints
 * g is an array of glen arrays of length m
 */
inline void applyFrobShort_2(int *x, int *idcsX, int lenX,
        int *x_mipo, int *idcsMipo, int lenMipo,
        int *g, int *idcsG, int *lenG, int glen,
        int *mats, int *idcsMats, int *lenMats, int frobpower,
        int *ret, int *idcsRet, int m, int charac, int *tmp, 
        int *tmp2, int *idcsTmp2,
        int *matmulCache, int *idcsMatmulCache, int *lenMatmulCache,
        bool *matmulCacheCalced,
        int *multTable, int *addTable){
    int mSize = m*m;
    int i,j,k,l;
    int lenRet = 0, lenTmp2;
    /*printf("applyFrobShort glen=%i x=",glen);printArr(x,m);*/
    for(i=0;i<glen;i++){
        /*printf("   gi=");printArr(g+i*m,m);*/
        if(lenG[i] == 0){
            /*printf("   is zero -> skip\n");*/
            continue;
        }
        if(matmulCacheCalced[i*frobpower] == 1){
            /*printf("   x^%i cached=",i*frobpower);*/
                                    /*printArr(matmulCache+m*(i*frobpower),m);*/
            j = m*(i*frobpower);
            k = i*m;
            lenTmp2 = multiplyPolyShort_2(
                    matmulCache+j, idcsMatmulCache+j, lenMatmulCache[i*frobpower],
                    g+k, idcsG+k, lenG[i],
                    tmp2, idcsTmp2, charac, multTable,addTable);
        }else{
            j = i*frobpower*mSize;
            k = i*frobpower*m; 
            lenTmp2 = matmulShort_2(
                    mats+j, idcsMats+j, lenMats+k,
                    x, idcsX, lenX,
                    tmp, idcsTmp2, m, charac,
                    multTable,addTable);
            lenMatmulCache[i*frobpower] = copyPoly(tmp, idcsTmp2, lenTmp2, 
                    matmulCache+k, idcsMatmulCache+k);
            matmulCacheCalced[i*frobpower] = 1;
            /*printf("   x^%i=",i*frobpower);printArr(tmp,m);*/
            j = i*m;
            lenTmp2 = multiplyPolyShort_2(tmp,idcsTmp2,lenTmp2,
                    g+j, idcsG+j, lenG[i],
                    tmp2, idcsTmp2, charac, multTable,addTable);
            /*printf("   gi*x^i=");printArr(tmp2,2*m);*/
            /*multiplyPolyShort(tmp,m, g+i*m, m, */
                    /*tmp2, 2*m, charac);*/
        }
        lenRet = moduloMonomShort_2(tmp2, idcsTmp2, lenTmp2, 
                x_mipo, idcsMipo, lenMipo, charac,multTable,addTable);
        /*printf("   gi*x^i mod=");printArr(tmp2,2*m);*/
        for(j=0;j<lenTmp2;j++){
            k = idcsTmp2[j];
            ret[k] = addTable[ ret[k] + tmp2[k] ];
        }
        k=0;
        for(j=m-1;j>=0;j--){
            if(ret[j] != 0){
                idcsRet[k] = j;
                k++;
            }
        }
        lenRet = k;
        /*printf("      => ret=");printArr(ret,m);*/
    }
    /*for(i=0;i<m;i++){*/
        /*ret[i] %= charac;*/
    /*}*/
}
inline void applyFrobShort_noCache(int *x, int *x_mipo, 
        int *g, int glen, int *gCoeffDegs,
        int *mats, int frobpower,
        int *ret, int m, int charac, int *tmp, int *tmp2,
        int *multTable, int *addTable){
    int mSize = m*m;
    int i,j,k,l;
    initPoly(ret,m);
    /*printf("applyFrobShort glen=%i x=",glen);printArr(x,m);*/
    for(i=0;i<glen;i++){
        /*printf("  i=%i   giDeg=%i gi=",i,gCoeffDegs[i]);printArr(g+i*m,m);*/
        if(allZero(g+i*m,m) == true){
            /*printf("   is zero -> skip\n");*/
            continue;
        }
        matmulShort(mats+i*frobpower*mSize, x, tmp, m, charac,
                multTable,addTable);
        /*printf("           x^q^i="); printArr(tmp,m);*/
        multiplyPolyShortKnownDeg(tmp,m, g+i*m, m, gCoeffDegs[i],
                tmp2, 2*m, charac, multTable,addTable);
        /*printf("        gi*x^q^i="); printArr(tmp2,2*m);*/
        moduloMonomShort(tmp2, 2*m, x_mipo, m+1, charac, multTable, addTable);
        /*printf("    gi*x^q^i mod="); printArr(tmp2,m);*/
        for(j=0;j<m;j++){
            ret[j] = addTable[ ret[j] + tmp2[j] ];
        }
    }
    /*printf("      -> ret=");printArr(ret,m);*/
    /*for(i=0;i<m;i++){*/
        /*ret[i] %= charac;*/
    /*}*/
}



//inline void testPolys(int *x, int *x_mipo, int decompCount,
//        int *polys, int *polysLen, int *polysCount, bool *evalToZero,
//        int *mats, int *frobPowers, 
//        int *ret, int m, int charac, int *tmp, int *tmp2){
//    int i,j;
//    int curDecompPosition = 0;
//    int curPolyPosition = 0;
//    int lastZeroPoly = 0;
//    int goodCounter = 0;
//    /*printf("testPolys for x="), printArr(x,m);*/
//    /*printf("polysCount="); printArr(polysCount,decompCount);*/
//
//    for(i=0;i<decompCount;i++){
//        /*printf("decomp: i=%i\n",i);*/
//        goodCounter = 0;
//        for(j=0;j<polysCount[i];j++){
//            /*printf("\ttest poly j=%i",j); */
//            applyFrob(x, x_mipo, 
//                    polys+curPolyPosition, polysLen[curDecompPosition+j],
//                    mats, frobPowers[curDecompPosition+j], 
//                    ret, m, charac, tmp, tmp2);
//            /*printf("\t=>ret=");printArr(ret,m);*/
//            if( allZero(ret,m) == evalToZero[curDecompPosition+j] ){
//                /*printf(" good\n");*/
//                goodCounter += 1;
//            }else{
//                /*printf(" not good\n");*/
//            }
//            curPolyPosition += m*polysLen[curDecompPosition+j];
//        }
//        /*printf("\t all %i tested => goodCounter=%i\n", polysCount[i],goodCounter);*/
//        if(goodCounter == polysCount[i]){
//            ret[0] = i;
//            return;
//        }
//        curDecompPosition += polysCount[i];
//    }
//    
//    ret[0] = -1;
//}


/**
 * test if x is completely normal.
 * polys are only cofactors of cyclotomic polynomial, so all must eval
 * to non zero
 */
inline void testCN(int *x, int *x_mipo, 
        int *polys, int *polysLen, int *polysCoeffDegs, int polysCount, 
        int *mats, int *frobPowers, 
        int *ret, int m, int charac, int *tmp, int *tmp2, int *tmp_x,
        int *matmulCache, bool *matmulCacheCalced){
    int i,j;
    int curPolyPosition = 0;
    int curPolyCoeffPosition = 0;
    
    /*printf("testCN x=");printArr(x,m);*/
    for(j=0;j<polysCount;j++){
        /*printf("    apply poly=");printArr(polys+curPolyPosition,m*polysLen[j]);*/
        /*printf("           degs=");*/
            /*printArr(polysCoeffDegs+curPolyCoeffPosition,polysLen[j]);*/
        applyFrob(x, x_mipo, 
                polys+curPolyPosition, polysLen[j],
                polysCoeffDegs+curPolyCoeffPosition,
                mats, frobPowers[j],
                ret, m, charac, tmp, tmp2, tmp_x,
                matmulCache, matmulCacheCalced);
        /*printf("             =>"); printArr(ret,m);*/
        if( allZero(ret,m) == true ){
            ret[0] = -1;
            return;
        }
        curPolyPosition += m*polysLen[j];
        curPolyCoeffPosition += polysLen[j];
    }
    
    ret[0] = 0;
    /*printf("\tret[0]=%i\n",ret[0]);*/
}

inline void testPolysShort(int *x, int *x_mipo, int decompCount,
        int *polys, int *polysLen, int *polysCoeffDegs, int *polysCount, 
        bool *evalToZero, int *mats, int *frobPowers, bool *toTestIndicator,
        int *ret, int m, int charac, int *tmp, int *tmp2,
        int *matmulCache, bool *matmulCacheCalced,
        int *multTable, int *addTable){
    int i,j;
    int curDecompPosition = 0;
    int curPolyPosition = 0;
    int curPolyCoeffPosition = 0;
    int lastZeroPoly = 0;
    int goodCounter = 0;
    /*if(toTestIndicator != 0){*/
        /*printf("testPolysShort toTestIndicator=");*/
        /*for(i=0;i<decompCount;i++) printf("%u ",toTestIndicator[i]);*/
    /*}*/
    /*printf("testPolysShort x=");printArr(x,m);*/

    for(i=0;i<decompCount;i++){
        if(toTestIndicator != 0 && toTestIndicator[i] == false){
            /*printf("i=%i -> continue\n",i);*/
            for(j=0;j<polysCount[i];j++){
                curPolyPosition += m*polysLen[curDecompPosition+j];
                curPolyCoeffPosition += polysLen[curDecompPosition+j];
            }
            curDecompPosition += polysCount[i];
            continue;
        }
        goodCounter = 0;
        for(j=0;j<polysCount[i];j++){
            /*printf("    apply poly=");printArr(polys+curPolyPosition,m*polysLen[curDecompPosition+j]);*/
            applyFrobShort(x, x_mipo, 
                    polys+curPolyPosition, polysLen[curDecompPosition+j],
                    polysCoeffDegs+curPolyCoeffPosition,
                    mats, frobPowers[curDecompPosition+j],
                    ret, m, charac, tmp, tmp2,
                    matmulCache, matmulCacheCalced,
                    multTable, addTable);
            /*printf("             =>"); printArr(ret,m);*/
            if( allZero(ret,m) == evalToZero[curDecompPosition+j] ){
                goodCounter += 1;
            }
            curPolyPosition += m*polysLen[curDecompPosition+j];
            curPolyCoeffPosition += polysLen[curDecompPosition+j];
        }
        if(goodCounter == polysCount[i]){
            ret[0] = i;
            /*printf("\tret[0]=%i\n",ret[0]);*/
            return;
        }
        curDecompPosition += polysCount[i];
    }
    
    ret[0] = -1;
    /*printf("\tret[0]=%i\n",ret[0]);*/
}


//make a linked list
struct Node {
    int *x;
    struct Node * next;
};

/**
 * appends element to end of root, where element is copied to new array
 */
inline struct Node *appendToEnd(struct Node *root, int * element, int elLen){
    struct Node *nextNode = root;
    if( nextNode != 0){
        while(nextNode->next != 0){
            nextNode = nextNode->next;
        }
        if( nextNode->x != 0){
            nextNode->next = malloc( sizeof(struct Node) );
            nextNode = nextNode->next;
        }
        if( nextNode != 0){
            nextNode->next = 0;
            nextNode->x = malloc(elLen*sizeof(int));
            copyArray(element,nextNode->x,elLen);
            return nextNode;
        }
    }
    return NULL;
}


inline void freeNode(struct Node* head){
    struct Node *next_n = NULL;
    struct Node *tmp_n = NULL;
    for(tmp_n=head; tmp_n !=NULL; ){
        next_n = tmp_n->next;
        /*printf("free node: %i ",n);*/
        free(tmp_n->x);
        /*printf(" data freed");*/
        free(tmp_n);
        /*printf(" tmp freed ");*/
        tmp_n = next_n;
        /*printf(" next=%i\n",n);*/
    }
    head = 0;
}

inline struct Node *appendNode(struct Node *curNode){
    curNode->next = malloc(sizeof(struct Node));
    curNode = curNode->next;
    curNode->x = 0;
    curNode->next = 0;
    return curNode;
}

inline void calcSubmoduleElements_FisNotPrime(struct Node *root, int *x, 
        int *x_mipo, 
        int maxLenPoly,
        int *genCounts, int curGen,
        int *polys, int *polysLen, int *polysCoeffDegs, int *polysCount,
        bool *evalToZero, int *mats, int matLen, int *frobPowers, 
        int *elementsF, int *elementsFDegs,
        int m, int charac, int q, 
        int *tmp_x, int *ret, int *tmp, int *tmp2, 
        int *matmulCache, bool *matmulCacheCalced,
        int *multTable, int *addTable){
    int i,j;
    int lenSize = maxLenPoly*m;
    int *curPoly = malloc( maxLenPoly*sizeof(int) );
    int *curFPoly = malloc( lenSize*sizeof(int) );
    int *polyCoeffDegs = malloc( maxLenPoly*sizeof(int) );
    struct Node *curRoot = root;
    copyArray(root->x,x,m);
    
    initPoly(curPoly, maxLenPoly);
    initPoly(curFPoly, lenSize);
    initPoly(polyCoeffDegs, maxLenPoly);
    curPoly[0] = 1;
    /*printf("calcSubmoduleElements_FisPrime: curGen=%i maxLenPoly=%i x=",curGen,maxLenPoly);printArr(x,m);*/
    int curLenPoly = 1;
    curPoly[0] = 2;
    if(q == 2 && maxLenPoly > 1){
        curLenPoly = 2;
        curPoly[0] = 0;
        curPoly[1] = 1;
    }
    if(q != 2 || maxLenPoly > 1){
        while(1==1){
            /*printf("  curLenPoly=%i curPoly=",curLenPoly);printArr(curPoly,maxLenPoly);*/
            // generate curElement
            for(i=0;i<curLenPoly;i++){
                copyArray(elementsF + m*curPoly[i], curFPoly+i*m, m);
                polyCoeffDegs[i] = elementsFDegs[curPoly[i]];
            }
            /*printf("  => curFPoly=");printArr(curFPoly,lenSize);*/
            /*printf("     polyCoeffDegs=");printArr(polyCoeffDegs,maxLenPoly);*/
            //apply Frobenius
            applyFrobShort_noCache(x, x_mipo,
                    curFPoly, curLenPoly, polyCoeffDegs,
                    mats, 1,
                    tmp_x, m, charac, tmp,tmp2, multTable, addTable);
            /*printf("          f(x)="); printArr(tmp_x,m);*/
            
            for(i=0;i<matLen;i++) matmulCacheCalced[i] = 0;
            testPolysShort(tmp_x,x_mipo,1,
                    polys,polysLen,polysCoeffDegs, polysCount, evalToZero,
                    mats, frobPowers, 0,
                    ret, m, charac, tmp, tmp2,
                    matmulCache,matmulCacheCalced,
                    multTable, addTable);
            /*printf("\tret[0] = %i\n",ret[0]);*/
            if( ret[0] != -1){
                /*printf("append tmp_x=");printArr(tmp_x,m);*/
                curRoot = appendToEnd(curRoot, tmp_x,m);
                genCounts[curGen] += 1;
            }

            //generate next element
            curPoly[0] += 1;
            if( curPoly[0] == q){
                for(i=0;i<maxLenPoly-1 && curPoly[i]==q;i++){
                    curPoly[i] = 0;
                    curPoly[i+1] += 1;
                }
                if(i+1>curLenPoly)
                    curLenPoly = i+1;
                if( curPoly[maxLenPoly-1]==q){
                    break;
                }
            }
        }
    }
    /*printf("genCounts[curGen] = %i",genCounts[curGen]);*/
    free(curPoly);
    free(curFPoly);
    free(polyCoeffDegs);
}

inline unsigned long long processLastSubmoduleAndTestPrimitivity_FisNotPrime(
        struct Node **roots, int *x,
        int *x_mipo, int decompCount,
        int maxLenPoly,
        int *genCounts, int curGen,
        int *polys, int *polysLen, int *polysCoeffDegs, int *polysCount,
        bool *evalToZero, int *mats, int matLen, int *frobPowers, 
        int *elementsF, int *elementsFDegs,
        int m, int charac, int q, 
        int *barFactors, int *lenBarFactors, int countBarFactors,
        int *biggestPrimeFactor, int lenBiggestPrimeFactor,
        int *matCharac,
        int *tmp_x, int *ret, int *tmp, int *tmp2, 
        int *matmulCache, bool *matmulCacheCalced,
        int *multTable, int *addTable){
   
    int lenSize = maxLenPoly*m;
    int *curPoly = malloc( maxLenPoly*sizeof(int) );
    int *curFPoly = malloc( lenSize*sizeof(int) );
    int *polyCoeffDegs = malloc( maxLenPoly*sizeof(int) );
    int *tmp_x2 = malloc( m*sizeof(int) );
    int *tmp_x3 = malloc( m*sizeof(int) );
    int * tmpRes = malloc( m*sizeof(int) );
    int * tmpRes2 = malloc( m*sizeof(int) );
    struct Node **curRoots = malloc(decompCount*sizeof(struct Node));
    
    int i,j;
    
    copyArray(roots[curGen]->x,x,m);
    initPoly(curPoly, maxLenPoly);
    initPoly(curFPoly, lenSize);
    curPoly[0] = 1;
    for(i=0;i<maxLenPoly;i++) polyCoeffDegs[i] = 0;
    int curLenPoly = 1;
    unsigned long long pcn = 0;
    while(1==1){
        // generate curElement
        for(i=0;i<curLenPoly;i++){
            copyArray(elementsF + m*curPoly[i], curFPoly+i*m, m);
            polyCoeffDegs[i] = elementsFDegs[curPoly[i]];
        }
        /*printf("  curLenPoly=%i curPoly=",curLenPoly);printArr(curPoly,maxLenPoly);*/
        /*printf("               curFPoly=");printArr(curFPoly,lenSize);*/
        //apply Frobenius
        applyFrobShort_noCache(x, x_mipo, curFPoly, curLenPoly, polyCoeffDegs,
                mats, 1, 
                tmp_x, m, charac, tmp,tmp2,multTable, addTable);
        /*printf("  f(x)="); printArr(tmp_x,m);*/
        
        for(i=0;i<matLen;i++) matmulCacheCalced[i] = 0;
        testPolysShort(tmp_x,x_mipo,1,
                polys,polysLen,polysCoeffDegs, polysCount, evalToZero,
                mats, frobPowers, 0,
                ret, m, charac, tmp, tmp2,
                matmulCache,matmulCacheCalced,
                multTable, addTable);
        /*printf(" ret[0]=%i\n",ret[0]);*/
        if( ret[0] != -1 ){
            /*printf("[");*/
            /*for(i=0;i<m-1;i++) printf("%i, ",tmp_x[i]);*/
            /*printf("%i],\n",tmp_x[m-1]);*/
            genCounts[curGen] += 1;
            //--------------------
            //test for primitivity
            for(i=0;i<decompCount;i++){
                curRoots[i] = roots[i];
            }
            if(decompCount > 1){
                while(1==1){
                    /*printf("\ttest primitivity: \n");*/
                    copyArray(curRoots[0]->x,tmp_x2,m);
                    /*printf("\t\tcurRoots[%i] = ",0); printArr(curRoots[0]->x,m);*/
                    for(i=1;i<decompCount-1;i++){
                        addPoly(curRoots[i]->x, tmp_x2, tmp_x2, m, charac,
                                multTable, addTable);
                        /*printf("\t\tcurRoots[%i] = ",i); printArr(curRoots[i]->x,m);*/
                    }
                    addPoly(tmp_x,tmp_x2,tmp_x2,m,charac, multTable,addTable);
                    /*printf("\t\ttmp_x = "); printArr(tmp_x,m);*/
                    /*for(i=0;i<m;i++)*/
                       /*tmp_x2[i] %= charac;*/
                    /*moduloPoly(tmp_x2,m,x_mipo,m+1,charac);*/
                    /*printf("\t\t=> tmp_x2=");printArr(tmp_x2,m);*/


                    //test primitivity
                    if(isPrimitive(tmp_x2,x_mipo,m,
                                barFactors,lenBarFactors,countBarFactors,
                                biggestPrimeFactor, lenBiggestPrimeFactor,
                                matCharac, charac,tmp_x3,
                                tmp,tmpRes,tmpRes2,tmp2,
                                multTable, addTable) == true){
                        /*printf("\t=> is primitive!\n");*/
                        pcn ++;
                    }else{
                    }

                    //nextElement
                    curRoots[0] = curRoots[0]->next;
                    if( curRoots[0] == 0 ){
                       for(i=0;i<decompCount-1 && curRoots[i]==0;i++){
                           curRoots[i] = roots[i];
                           curRoots[i+1] = curRoots[i+1]->next;
                       }
                    }
                    if( curRoots[decompCount-1] == 0){
                       break;
                    }
                }
            }else{
                if(isPrimitive(tmp_x,x_mipo,m,
                            barFactors,lenBarFactors,countBarFactors,
                            biggestPrimeFactor, lenBiggestPrimeFactor,
                            matCharac, charac,
                            tmp_x3,tmp,tmpRes,tmpRes2,tmp2,
                            multTable, addTable) == true){
                    /*printf("\t=> is primitive!\n");*/
                    pcn ++;
                }
            }
            //-------------------
        }

        //generate next element
        curPoly[0] += 1;
        if( curPoly[0] == q){
            for(i=0;i<maxLenPoly-1 && curPoly[i]==q;i++){
                curPoly[i] = 0;
                curPoly[i+1] += 1;
            }
            if(i+1>curLenPoly)
                curLenPoly = i+1;
            if( curPoly[maxLenPoly-1]==q){
                break;
            }
        }
    }
    free(curRoots);
    free(curPoly);
    free(curFPoly);
    free(polyCoeffDegs);
    free(tmp_x2);
    free(tmp_x3);
    free(tmpRes);
    free(tmpRes2);
    genCounts[curGen]--;
    return pcn;
}


inline void calcSubmoduleElements_FisPrime(struct Node *root, int *x, 
        int *x_mipo, 
        int maxLenPoly,
        int *genCounts, int curGen,
        int *polys, int *polysLen, int *polysCoeffDegs, int *polysCount,
        bool *evalToZero, int *mats, int matLen, int *frobPowers, 
        int m, int charac, int q, 
        int *tmp_x, int *ret, int *tmp, int *tmp2, 
        int *matmulCache, bool *matmulCacheCalced,
        int *multTable, int *addTable){
    int i,j;
    int lenSize = maxLenPoly*m;
    int *curPoly = malloc( maxLenPoly*m*sizeof(int) );
    int *polyCoeffDegs = malloc( maxLenPoly*sizeof(int) );
    struct Node *curRoot = root;
    copyArray(root->x,x,m);
    
    initPoly(curPoly, lenSize);
    curPoly[0] = 1;
    for(i=0;i<maxLenPoly;i++) polyCoeffDegs[i] = 0;
    /*printf("calcSubmoduleElements_FisPrime: curGen=%i maxLenPoly=%i x=",curGen,maxLenPoly);printArr(x,m);*/
    int curLenPoly = 1;
    curPoly[0] = 2;
    if(charac == 2 && maxLenPoly > 1){
        curLenPoly = 2;
        curPoly[0] = 0;
        curPoly[m] = 1;
    }
    if(charac != 2 || maxLenPoly > 1){
        while(1==1){
            /*printf("  curLenPoly=%i curPoly=",curLenPoly);printArr(curPoly,lenSize);*/
            // generate curElement
            applyFrobShort_noCache(x, x_mipo, curPoly, curLenPoly, polyCoeffDegs,
                    mats, 1,
                    tmp_x, m, charac, tmp,tmp2, multTable, addTable);
            /*printf("          f(x)="); printArr(tmp_x,m);*/
            
            for(i=0;i<matLen;i++) matmulCacheCalced[i] = 0;
            testPolysShort(tmp_x,x_mipo,1,
                    polys,polysLen,polysCoeffDegs, polysCount, evalToZero,
                    mats, frobPowers, 0,
                    ret, m, charac, tmp, tmp2,
                    matmulCache,matmulCacheCalced,
                    multTable, addTable);
            /*printf("\tret[0] = %i\n",ret[0]);*/
            if( ret[0] != -1){
                curRoot = appendToEnd(curRoot, tmp_x,m);
                genCounts[curGen] += 1;
           }

            //generate next element
            curPoly[0] += 1;
            if( curPoly[0] == charac ){
                for(i=0;i<lenSize-m && curPoly[i]==charac;i+=m){
                    curPoly[i] = 0;
                    curPoly[i+m] += 1;
                }
                j = i/m+1;
                if(j>curLenPoly)
                    curLenPoly = j;
                if( curPoly[lenSize-m]==charac ){
                    break;
                }
            }
        }
    }
    /*printf("genCounts[curGen] = %i",genCounts[curGen]);*/
    free(curPoly);
    free(polyCoeffDegs);
}

inline unsigned long long processLastSubmoduleAndTestPrimitivity_FisPrime(
        struct Node **roots, int *x,
        int *x_mipo, int decompCount,
        int maxLenPoly,
        int *genCounts, int curGen,
        int *polys, int *polysLen, int *polysCoeffDegs, int *polysCount,
        bool *evalToZero, int *mats, int matLen, int *frobPowers, 
        int m, int charac, int q, 
        int *barFactors, int *lenBarFactors, int countBarFactors,
        int *biggestPrimeFactor, int lenBiggestPrimeFactor,
        int *matCharac,
        int *tmp_x, int *ret, int *tmp, int *tmp2, 
        int *matmulCache, bool *matmulCacheCalced,
        int *multTable, int *addTable){
   
    int *curPoly = malloc( maxLenPoly*m*sizeof(int) );
    int *polyCoeffDegs = malloc( maxLenPoly*sizeof(int) );
    int *tmp_x2 = malloc( m*sizeof(int) );
    int *tmp_x3 = malloc( m*sizeof(int) );
    int * tmpRes = malloc( m*sizeof(int) );
    int * tmpRes2 = malloc( m*sizeof(int) );
    struct Node **curRoots = malloc(decompCount*sizeof(struct Node));
    
    int i,j;
    int lenSize = maxLenPoly*m;
    
    copyArray(roots[curGen]->x,x,m);
    initPoly(curPoly, lenSize);
    curPoly[0] = 1;
    for(i=0;i<maxLenPoly;i++) polyCoeffDegs[i] = 0;
    int curLenPoly = 1;
    unsigned long long pcn = 0;
    /*printf("processLastSubmoduleAndTestPrimitivity curGen = %i, x=",curGen);*/
    /*printArr(x,m);*/
    /*printf("roots: \n");*/
    /*for(i=0;i<decompCount;i++){*/
        /*curRoots[i] = roots[i];*/
        /*printf("\ti=%i\n",i);*/
        /*while(curRoots[i] != 0){*/
            /*printf("\t\t");printArr(curRoots[i]->x,m);*/
            /*curRoots[i] = curRoots[i]->next;*/
        /*}*/
    /*}*/
    while(1==1){
        /*printf("  curLenPoly=%i curPoly=",curLenPoly);printArr(curPoly,lenSize);*/
        // generate curElement
        applyFrobShort_noCache(x, x_mipo, curPoly, curLenPoly, polyCoeffDegs,
                mats, 1, 
                tmp_x, m, charac, tmp,tmp2, multTable, addTable);
        /*printf("  f(x)="); printArr(tmp_x,m);*/
        
        for(i=0;i<matLen;i++) matmulCacheCalced[i] = 0;
        testPolysShort(tmp_x,x_mipo,1,
                polys,polysLen,polysCoeffDegs, polysCount, evalToZero,
                mats, frobPowers, 0,
                ret, m, charac, tmp, tmp2,
                matmulCache,matmulCacheCalced,
                multTable, addTable);
        if( ret[0] != -1 ){
            genCounts[curGen] += 1;
            //--------------------
            //test for primitivity
            for(i=0;i<decompCount;i++){
                curRoots[i] = roots[i];
            }
            if(decompCount > 1){
                while(1==1){
                    /*printf("\ttest primitivity: \n");*/
                    copyArray(curRoots[0]->x,tmp_x2,m);
                    /*printf("\t\tcurRoots[%i] = ",0); printArr(curRoots[0]->x,m);*/
                    for(i=1;i<decompCount-1;i++){
                        addPoly(curRoots[i]->x, tmp_x2, tmp_x2, m, charac,
                                multTable,addTable);
                        /*printf("\t\tcurRoots[%i] = ",i); printArr(curRoots[i]->x,m);*/
                    }
                    addPoly(tmp_x,tmp_x2,tmp_x2,m,charac, multTable,addTable);
                    /*printf("\t\ttmp_x = "); printArr(tmp_x,m);*/
                    /*for(i=0;i<m;i++)*/
                       /*tmp_x2[i] %= charac;*/
                    /*moduloPoly(tmp_x2,m,x_mipo,m+1,charac);*/
                    /*printf("\t\t=> tmp_x2=");printArr(tmp_x2,m);*/

                    //test primitivity
                    if(isPrimitive(tmp_x2,x_mipo,m,
                                barFactors,lenBarFactors,countBarFactors,
                                biggestPrimeFactor, lenBiggestPrimeFactor,
                                matCharac, charac,tmp_x3,
                                tmp,tmpRes,tmpRes2,tmp2,
                                multTable, addTable) == true){
                        /*printf("\t=> is primitive!\n");*/
                        pcn ++;
                    }

                    //nextElement
                    curRoots[0] = curRoots[0]->next;
                    if( curRoots[0] == 0 ){
                       for(i=0;i<decompCount-1 && curRoots[i]==0;i++){
                           curRoots[i] = roots[i];
                           curRoots[i+1] = curRoots[i+1]->next;
                       }
                    }
                    if( curRoots[decompCount-1] == 0){
                       break;
                    }
                }
            }else{
                if(isPrimitive(tmp_x,x_mipo,m,
                            barFactors,lenBarFactors,countBarFactors,
                            biggestPrimeFactor, lenBiggestPrimeFactor,
                            matCharac, charac,
                            tmp_x3,tmp,tmpRes,tmpRes2,tmp2,
                            multTable, addTable) == true){
                    /*printf("\t=> is primitive!\n");*/
                    pcn ++;
                }
            }
            //-------------------
        }

        //generate next element
        curPoly[0] += 1;
        if( curPoly[0] == charac ){
            for(i=0;i<lenSize-m && curPoly[i]==charac;i+=m){
                curPoly[i] = 0;
                curPoly[i+m] += 1;
            }
            j = i/m+1;
            if(j>curLenPoly)
                curLenPoly = j;
            if( curPoly[lenSize-m]==charac ){
                break;
            }
        }
    }
    free(curRoots);
    free(curPoly);
    free(polyCoeffDegs);
    free(tmp_x2);
    free(tmp_x3);
    free(tmpRes);
    free(tmpRes2);
    genCounts[curGen]--;
    return pcn;
}

unsigned long long processFFElements_useGens( int *x_mipo, int decompCount,
        int *polys, int *polysLen, int *polysCoeffDegs, int *polysCount, 
        bool *evalToZero, int *mats, int matLen, int *frobPowers, 
        int *genCounts, int m, int charac, int q, 
        int *barFactors, int *lenBarFactors, int countBarFactors,
        int *biggestPrimeFactor, int lenBiggestPrimeFactor,
        int *matCharac, int *elementsF, int *elementsFDegs,
        int *multTable, int initialMultShift, 
        int *addTable, int initialAddShift){
    time_t TIME = time(NULL);
    int i,j;

    multTable += initialMultShift;
    addTable += initialAddShift;

    int * x = malloc( m*sizeof(int) );
    int * ret = malloc( m*sizeof(int) );
    int * tmp = malloc( m*sizeof(int) );
    int * tmp_x = malloc( m*sizeof(int) );
    int * tmp2 = malloc( 2*m*sizeof(int) );
    int * matmulCache = malloc(matLen*m*sizeof(int));
    bool * matmulCacheCalced = malloc(matLen*sizeof(bool));
    bool * toTestIndicator = malloc (decompCount*sizeof(bool));
    int foundCounter = 0;


    unsigned long long pcn = 0;

    initPoly(x,m);
    initPoly(genCounts,decompCount);

    struct Node **roots = malloc( decompCount*sizeof(struct Node) );
    struct Node **curRoots = malloc(decompCount*sizeof(struct Node));
    for(i=0;i<decompCount;i++){
        roots[i] = malloc( sizeof(struct Node) );
        roots[i]->x = 0;
        roots[i]->next = 0;
        curRoots[i] = roots[i];
        toTestIndicator[i] = true;
    }

    
    while(1==1){
        for(i=0;i<matLen;i++) matmulCacheCalced[i] = 0;
        testPolysShort(x,x_mipo,decompCount,
                polys,polysLen,polysCoeffDegs, polysCount,evalToZero,
                mats, frobPowers, toTestIndicator,
                ret, m, charac, tmp, tmp2,
                matmulCache,matmulCacheCalced,
                multTable, addTable);
        if( ret[0] != -1){
            /*printf("ret[0] = %i\n",ret[0]);*/
            if(toTestIndicator[ret[0]] == true){
                /*printf("found gen: %i -> x=",ret[0]); printArr(x,m);*/
                genCounts[ret[0]] += 1;
                appendToEnd(roots[ret[0]], x, m);
                /*decodeArr(curRoots[ret[0]]->x,x,m,shiftSize);*/
                foundCounter ++;
                toTestIndicator[ret[0]] = false;
            }
            if(foundCounter == decompCount)
                break;
        }
        //generate next element
        x[0] += 1;
        if( x[0] == charac ){
            for(i=0;i<m-1 && x[i]==charac;i++){
                x[i] = 0;
                x[i+1] += 1;
            }
            if( x[m-1]==charac ){
                break;
            }
        }
    }
    printf("finding time: %.2f\n", (double)(time(NULL)-TIME));
    
    // process found elements
    int curDecompPosition = 0;
    int curPolyPosition = 0;
    int curPolyCoeffPosition = 0;

   for(i=0;i<decompCount-1;i++){
       if(charac == q){
           calcSubmoduleElements_FisPrime(roots[i],
                   x, x_mipo, polysLen[curDecompPosition]-1,
                   genCounts, i,
                   polys+curPolyPosition, polysLen+curDecompPosition,
                   polysCoeffDegs+curPolyCoeffPosition,
                   polysCount+i, evalToZero+curDecompPosition,
                   mats, matLen, frobPowers+curDecompPosition,
                   m, charac, q,
                   tmp_x, ret, tmp, tmp2,
                   matmulCache, matmulCacheCalced,
                   multTable, addTable);
       }else{
           calcSubmoduleElements_FisNotPrime(roots[i],
                   x, x_mipo, polysLen[curDecompPosition]-1,
                   genCounts, i,
                   polys+curPolyPosition, polysLen+curDecompPosition,
                   polysCoeffDegs+curPolyCoeffPosition,
                   polysCount+i, evalToZero+curDecompPosition,
                   mats, matLen, frobPowers+curDecompPosition,
                   elementsF, elementsFDegs,
                   m, charac, q,
                   tmp_x, ret, tmp, tmp2,
                   matmulCache, matmulCacheCalced,
                   multTable, addTable);
       }

       for(j=0;j<polysCount[i];j++){
           curPolyPosition += m*polysLen[curDecompPosition+j];
           curPolyCoeffPosition += polysLen[curDecompPosition+j];
       }
       curDecompPosition += polysCount[i];
   }
   printf("all not last time: %.2f\n", (double)(time(NULL)-TIME));

    // calc last dynamically and test for primitivity
    if(charac == q){
        pcn = processLastSubmoduleAndTestPrimitivity_FisPrime(
                roots, x,
                x_mipo, decompCount,
                polysLen[curDecompPosition]-1,
                genCounts, decompCount-1,
                polys+curPolyPosition, polysLen+curDecompPosition,
                polysCoeffDegs+curPolyCoeffPosition,
                polysCount+i, evalToZero+curDecompPosition,
                mats, matLen, frobPowers+curDecompPosition,
                m, charac, q,
                barFactors, lenBarFactors, countBarFactors,
                biggestPrimeFactor, lenBiggestPrimeFactor,
                matCharac,
                tmp_x, ret, tmp, tmp2,
                matmulCache, matmulCacheCalced,
                multTable, addTable);
    }else{
        pcn = processLastSubmoduleAndTestPrimitivity_FisNotPrime(
                roots, x,
                x_mipo, decompCount,
                polysLen[curDecompPosition]-1,
                genCounts, decompCount-1,
                polys+curPolyPosition, polysLen+curDecompPosition,
                polysCoeffDegs+curPolyCoeffPosition,
                polysCount+i, evalToZero+curDecompPosition,
                mats, matLen, frobPowers+curDecompPosition,
                elementsF, elementsFDegs,
                m, charac, q,
                barFactors, lenBarFactors, countBarFactors,
                biggestPrimeFactor, lenBiggestPrimeFactor,
                matCharac,
                tmp_x, ret, tmp, tmp2,
                matmulCache, matmulCacheCalced,
                multTable, addTable);
    }


    for(i=0;i<decompCount;i++){
        freeNode(roots[i]);
    }
    free(roots);
    free(curRoots);
    free(x);
    free(tmp);
    free(tmp2);
    free(tmp_x);
    free(ret);
    free(matmulCache);
    free(matmulCacheCalced);
    free(toTestIndicator);
    
    printf("total time: %.2f\n", (double)(time(NULL)-TIME));
    return pcn;
}


//unsigned long long processFFElements( int *x_mipo, int decompCount,
//        int *polys, int *polysLen, int *polysCoeffDegs, int *polysCount, 
//        bool *evalToZero, int *mats, int matLen, int *frobPowers, 
//        int *genCounts, int m, int charac, int shiftSize,
//        int *barFactors, int *lenBarFactors, int countBarFactors,
//        int *biggestPrimeFactor, int lenBiggestPrimeFactor,
//        int *matCharac){
//    time_t TIME = time(NULL);
//    int i,j;
//    
//    int * x = malloc( m*sizeof(int) );
//    int * ret = malloc( m*sizeof(int) );
//    int * tmp = malloc( m*sizeof(int) );
//    int * tmp_x = malloc( m*sizeof(int) );
//    int * tmpRes = malloc( m*sizeof(int) );
//    int * tmpRes2 = malloc( m*sizeof(int) );
//    int * tmp2 = malloc( 2*m*sizeof(int) );
//    int * matmulCache = malloc(matLen*m*sizeof(int));
//    bool * matmulCacheCalced = malloc(matLen*sizeof(bool));
//
//
//    unsigned long long pcn = 0;
//
//    initPoly(x,m);
//    initPoly(genCounts,decompCount);
//
//    struct Node **roots = malloc( decompCount*sizeof(struct Node) );
//    struct Node **curRoots = malloc(decompCount*sizeof(struct Node));
//    for(i=0;i<decompCount;i++){
//        roots[i] = malloc( sizeof(struct Node) );
//        roots[i]->x = 0;
//        roots[i]->next = 0;
//        curRoots[i] = roots[i];
//    }
//
//    
//    while(1==1){
//        for(i=0;i<matLen;i++) matmulCacheCalced[i] = 0;
//        testPolysShort(x,x_mipo,decompCount,
//                polys,polysLen,polysCoeffDegs, polysCount,evalToZero,
//                mats, frobPowers, 0,
//                ret, m, charac, tmp, tmp2,
//                matmulCache,matmulCacheCalced);
//        /*for(i=0;i<matLen;i++)*/
//            /*printf("%i ",matmulCacheCalced[i]);*/
//        /*printf("\n");*/
//        if( ret[0] != -1){
//            genCounts[ret[0]] += 1;
//            curRoots[ret[0]] = appendToEnd(curRoots[ret[0]], x, m);
//        }
//        //generate next element
//        x[0] += 1;
//        if( x[0] == charac ){
//            for(i=0;i<m-1 && x[i]==charac;i++){
//                x[i] = 0;
//                x[i+1] += 1;
//            }
//            if( x[m-1]==charac ){
//                break;
//            }
//        }
//    }
//    printf("CN time: %.2f\n", (double)(time(NULL)-TIME));
//
//    for(i=0;i<decompCount;i++){
//        curRoots[i] = roots[i];
//    }
//    while(1==1){
//        copyArray(curRoots[0]->x, x, m);
//        for(i=1;i<decompCount;i++){
//            addPoly(curRoots[i]->x, x, x, m, charac);
//        }
//        for(i=0;i<m;i++)
//            x[i] %= charac;
//        moduloPoly(x,m,x_mipo,m+1,charac);
//
//        //test primitivity
//        if(isPrimitive(x,x_mipo,m,barFactors,lenBarFactors,countBarFactors,
//                    biggestPrimeFactor, lenBiggestPrimeFactor,
//                    matCharac, charac,tmp_x,tmp,tmpRes,tmpRes2,tmp2) == true){
//            pcn ++;
//        }
//
//        //nextElement
//        curRoots[0] = curRoots[0]->next;
//        if( curRoots[0] == 0 ){
//            for(i=0;i<decompCount-1 && curRoots[i]==0;i++){
//                curRoots[i] = roots[i];
//                curRoots[i+1] = curRoots[i+1]->next;
//            }
//        }
//        if( curRoots[decompCount-1] == 0){
//            break;
//        }
//    }
//
//    for(i=0;i<decompCount;i++){
//        freeNode(roots[i]);
//    }
//    free(roots);
//    free(curRoots);
//    free(tmp2);
//    free(tmp);
//    free(tmpRes);
//    free(tmpRes2);
//    free(tmp_x);
//    free(ret);
//    free(matmulCache);
//    free(matmulCacheCalced);
//    
//    printf("total time: %.2f\n", (double)(time(NULL)-TIME));
//    return pcn;
//}
//
//
//double eta_processFFElements( int *x_mipo, int decompCount,
//        int *polys, int *polysLen, int *polysCoeffDegs, int *polysCount, bool *evalToZero,
//        int *mats, int matLen, int *frobPowers, 
//        int *genCounts, int m, int charac, int shiftSize){
//    struct timeval TIME1, TIME2;
//    int i,j;
//    
//    
//    int * x = malloc( m*sizeof(int) );
//    int * ret = malloc( m*sizeof(int) );
//    int * tmp = malloc( m*sizeof(int) );
//    int * tmp2 = malloc( 2*m*sizeof(int) );
//    int * matmulCache = malloc(matLen*m*sizeof(int));
//    bool * matmulCacheCalced = malloc(matLen*sizeof(bool));
//
//    initPoly(x,m);
//    initPoly(genCounts,decompCount);
//
//    struct Node **roots = malloc( decompCount*sizeof(struct Node) );
//    struct Node **curRoots = malloc(decompCount*sizeof(struct Node));
//    for(i=0;i<decompCount;i++){
//        roots[i] = malloc( sizeof(struct Node) );
//        roots[i]->x = 0;
//        roots[i]->next = 0;
//        curRoots[i] = roots[i];
//    }
//
//    int counter = 0;
//    gettimeofday(&TIME1,NULL);
//    
//    while(1==1){
//        for(i=0;i<matLen;i++) matmulCacheCalced[i] = 0;
//        testPolysShort(x,x_mipo,decompCount,
//                polys,polysLen,polysCoeffDegs, polysCount,evalToZero,
//                mats, frobPowers, 0,
//                ret, m, charac, tmp, tmp2,
//                matmulCache, matmulCacheCalced);
//        if( ret[0] != -1){
//            genCounts[ret[0]] += 1;
//            curRoots[ret[0]] = appendToEnd(curRoots[ret[0]], x, m);
//        }
//        //generate next element
//        x[0] += 1;
//        if( x[0] == charac ){
//            for(i=0;i<m-1 && x[i]==charac;i++){
//                x[i] = 0;
//                x[i+1] += 1;
//            }
//            if( x[m-1]==charac ){
//                break;
//            }
//        }
//        counter++;
//        if(counter == 1000)
//            break;
//    }
//
//    gettimeofday(&TIME2,NULL);
//    
//    for(i=0;i<decompCount;i++){
//        /*printf("free root %i\n",i);*/
//        freeNode(roots[i]);
//    }
//    free(roots);
//    /*printf("roots freed!\n");*/
//    free(curRoots);
//    /*printf("curRoots freed!\n");*/
//    free(tmp2);
//    /*printf("tmp2 freed!\n");*/
//    free(tmp);
//    /*printf("tmp freed!\n");*/
//    free(ret);
//    /*printf("ret freed!\n");*/
//    free(x);
//    /*printf("x freed!\n");*/
//    free(matmulCache);
//    free(matmulCacheCalced);
//    
//    double timediff = (TIME2.tv_sec - TIME1.tv_sec +
//         ((double)(TIME2.tv_usec - TIME1.tv_usec))/1000000.0);
//    return 4*timediff *pow((double)charac,(double)m)/counter;
//}


///**
// * returns the next CN element
// */
//void findAnyPCN(int * x, int *x_mipo, 
//        int *polys, int *polysLen, int *polysCount, bool *evalToZero,
//        int *mats, int *frobPowers, 
//        int m, int charac, 
//        int *barFactors, int *lenBarFactors, int countBarFactors,
//        int *biggestPrimeFactor, int lenBiggestPrimeFactor,
//        int *matCharac){
//        /*int *ret, int *tmp, int * tmp2){*/
//    time_t TIME = time(NULL);
//    int i,j;
//    
//    
//    int * ret = malloc( m*sizeof(int) );
//    int * tmp = malloc( m*sizeof(int) );
//    int * tmp_x = malloc( m*sizeof(int) );
//    int * tmpRes = malloc( m*sizeof(int) );
//    int * tmpRes2 = malloc( m*sizeof(int) );
//    int * tmp2 = malloc( 2*m*sizeof(int) );
//
//    bool pcnFound = false;
//    
//    while(1==1){
//        //generate next element
//        x[0] += 1;
//        if( x[0] == charac ){
//            for(i=0;i<m-1 && x[i]==charac;i++){
//                x[i] = 0;
//                x[i+1] += 1;
//            }
//            if( x[m-1]==charac ){
//                x[0] = -1;
//                return;
//            }
//        }
//        testPolys(x,x_mipo,1,
//                polys,polysLen,polysCount,evalToZero,
//                mats, frobPowers,
//                ret, m, charac, tmp, tmp2);
//        if( ret[0] != -1){
//            if(isPrimitive(x,x_mipo,m,barFactors,lenBarFactors,countBarFactors,
//                        biggestPrimeFactor, lenBiggestPrimeFactor,
//                        matCharac,charac, tmp_x,tmp,tmpRes,tmpRes2,tmp2) == true){
//                pcnFound = true;
//                break;
//            }
//        }
//    }
//    free(ret);
//    free(tmp);
//    free(tmpRes);
//    free(tmpRes2);
//    free(tmp_x);
//    free(tmp2);
//
//    if(pcnFound == false){
//        x[0] = -1;
//    }
//
//    printf("C time: %.2f\n", (double)(time(NULL)-TIME));
//}


bool findAnyPCN_useGen(int * x, int * generator, int *x_mipo, 
        int *polys, int *polysLen, int *polysCoeffDegs, 
        int polysCount, 
        int *mats, int matLen, int *frobPowers, 
        int m, int charac, 
        int *multTable, int initialMultShift, 
        int *addTable, int initialAddShift,
        int *powerTable, int lenPowerTable){
    time_t TIME = time(NULL);
    int i,j;
    
    multTable += initialMultShift;
    addTable += initialAddShift;
    
    int * ret = malloc( m*sizeof(int) );
    int * tmp = malloc( m*sizeof(int) );
    int * tmp_x = malloc( m*sizeof(int) );
    int * tmp2 = malloc( 2*m*sizeof(int) );
    int * matmulCache = malloc(matLen*m*sizeof(int));
    bool * matmulCacheCalced = malloc(matLen*sizeof(bool));

    bool pcnFound = false;

    // the generator
    for(j=0;j<m;j++) x[j] = generator[j];
    /*printf("gen=");printArr(x,m);*/
    
    for(i=0;i<lenPowerTable;i++){
        if(powerTable[i] != 0){
            for(j=0;j<m;j++) tmp_x[j] = generator[j];
            /*powerPolyInt(tmp_x, x_mipo, ret, m, powerTable[i], charac, tmp2);*/
            powerPolyInt(tmp_x, x_mipo, ret, m, 
                    powerTable[i], charac, tmp2);
            /*printf("gen^%i=",powerTable[i]);printArr(ret,m);*/

            multiplyPoly(x,m,ret,m,tmp2,2*m,charac);
            moduloMonom(tmp2,2*m, x_mipo,m+1,charac);
            /*multiplyPoly(x,m, ret,m, tmp2, 2*m, charac);*/
            /*moduloPoly(tmp2, 2*m, x_mipo, m+1, charac);*/
            copyArray(tmp2,x,m);
        }
        for(j=0;j<matLen;j++) matmulCacheCalced[j] = false;

        /*printf("test x=");printArr(x,m);*/
        testCN(x,x_mipo, polys,polysLen,polysCoeffDegs,polysCount,
                mats,frobPowers,
                ret,m,charac,tmp,tmp2, tmp_x,
                matmulCache,matmulCacheCalced);
        /*printf("    =>ret[0]=%i",ret[0]);*/

        /*testPolys(x,x_mipo,1,*/
                /*polys,polysLen,polysCount,evalToZero,*/
                /*mats, frobPowers,*/
                /*ret, m, charac, tmp, tmp2);*/
        if( ret[0] != -1){
            for(j=0;j<m;j++){
                if( x[j]<0 ) x[j] += charac;
            }
            pcnFound = true;
            break;
        }
    }
    free(matmulCacheCalced);
    free(matmulCache);
    free(ret);
    free(tmp);
    free(tmp_x);
    free(tmp2);

    /*printf("C time: %.2f\n", (double)(time(NULL)-TIME));*/
    return pcnFound;
}



void test1(){
    int mats[] =
        {1,0,0,0,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,0,0,
        0,0,1,0,0,0,0,0,0,0,
        0,0,0,1,0,0,0,0,0,0,
        0,0,0,0,1,0,0,0,0,0,
        0,0,0,0,0,1,0,0,0,0,
        0,0,0,0,0,0,1,0,0,0,
        0,0,0,0,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,1,0,
        0,0,0,0,0,0,0,0,0,1,
        1,0,0,0,0,1,0,1,0,0,
        0,0,0,0,0,1,0,1,1,1,
        0,1,0,0,0,1,1,1,0,1,
        0,0,0,0,0,1,1,1,0,0,
        0,0,1,0,0,0,1,1,0,1,
        0,0,0,0,0,1,1,0,1,0,
        0,0,0,1,0,1,0,0,0,1,
        0,0,0,0,0,0,1,1,1,0,
        0,0,0,0,1,0,1,0,0,0,
        0,0,0,0,0,0,0,1,1,1,
        1,0,0,0,0,0,0,0,0,0,
        0,0,0,0,1,1,1,0,1,1,
        0,0,0,1,0,1,0,1,0,1,
        0,0,0,1,0,0,0,1,0,1,
        0,1,0,1,0,0,0,1,0,1,
        0,0,0,1,1,0,0,0,1,1,
        0,0,0,0,0,0,0,0,0,1,
        0,0,0,1,1,1,0,1,1,1,
        0,0,1,1,0,1,1,1,0,0,
        0,0,0,0,1,0,0,0,0,1,
        1,0,0,0,0,1,0,1,0,0,
        0,0,1,1,1,0,1,0,0,1,
        0,0,0,0,0,0,1,1,1,1,
        0,0,0,0,0,1,0,1,0,1,
        0,0,0,0,0,0,0,0,1,0,
        0,0,1,0,1,1,1,1,1,0,
        0,0,0,0,0,0,0,1,1,1,
        0,0,1,0,1,0,1,0,1,0,
        0,1,0,1,0,0,0,1,0,0,
        0,0,1,0,0,0,1,0,1,0,
        1,0,0,0,0,0,0,0,0,0,
        0,1,1,1,0,1,1,0,1,0,
        0,0,0,1,1,1,0,0,0,0,
        0,0,0,0,0,1,0,0,1,1,
        0,0,0,0,1,0,1,0,0,0,
        0,1,1,1,1,1,1,1,0,1,
        0,0,0,0,1,0,0,0,0,1,
        0,1,1,1,1,0,1,0,0,1,
        0,0,0,0,0,0,0,1,0,1,
        0,1,0,1,1,0,0,1,0,0,
        1,0,0,0,0,1,0,1,0,0,
        0,1,0,1,1,1,0,1,0,1,
        0,0,1,0,0,0,1,0,1,1,
        0,0,0,0,1,1,0,1,0,1,
        0,0,1,1,0,1,1,1,0,0,
        0,1,1,1,0,1,1,0,0,1,
        0,0,1,0,0,0,1,0,1,0,
        0,1,1,1,0,0,1,1,0,1,
        0,0,0,0,0,0,1,0,0,1,
        0,0,1,0,0,0,1,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        0,0,1,0,0,1,0,1,0,1,
        0,1,0,1,1,0,0,0,1,1,
        0,0,1,0,0,1,1,1,1,0,
        0,1,0,1,0,0,0,1,0,0,
        0,1,0,1,0,1,1,0,1,0,
        0,1,0,1,1,0,0,1,0,0,
        0,1,0,1,0,0,1,1,1,0,
        0,0,0,1,0,1,0,1,1,0,
        0,1,0,1,0,0,1,1,0,0,
        1,0,0,0,0,1,0,1,0,0,
        0,1,0,0,0,0,1,1,1,0,
        0,0,1,0,1,0,1,0,0,1,
        0,1,0,1,1,1,0,0,0,0,
        0,0,0,0,0,0,0,1,0,1,
        0,0,0,1,1,0,1,0,0,0,
        0,0,1,0,0,0,1,0,0,0,
        0,0,0,1,1,1,1,1,0,0,
        0,0,0,0,1,0,0,0,0,0,
        0,0,0,1,0,1,0,1,0,0,
        1,0,0,0,0,0,0,0,0,0,
        0,0,0,1,1,0,0,0,0,0,
        0,1,1,1,0,0,0,1,1,0,
        0,0,1,0,0,1,1,1,0,0,
        0,0,0,0,0,0,1,0,0,1,
        0,0,1,1,0,0,0,0,0,0,
        0,1,0,1,0,0,1,1,0,0,
        0,0,1,1,0,1,0,1,0,0,
        0,0,1,0,0,0,1,1,0,1,
        0,0,0,0,0,0,1,0,0,0};
    int polys[] = 
        {1,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        1,1,0,1,0,1,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,
        0,1,0,1,0,1,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0};
    int polysCoeffDegs[] = {0, -1, 0, 0, 0, 0, 0, 0, 0, -1, 0, -1, 0, -1, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 5, 0};
    int matLen = 10;
    int polysLen[] = {3, 2, 2, 1, 9, 5, 5, 3, 3};
    bool evalToZero[] = {1, 0, 1, 0, 1, 0, 1, 0, 0};
    int frobPowers[] = {1, 1, 2, 2, 1, 1, 2, 2, 2};
    int polysCount[] = {4, 5};
    int xmipo[] = {1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1};
    int decompCount = 2;
    int m = 10;
    int charac = 2;
    int q = 2;
    int shiftSize = 1;
    int barFactors[] = {1, 0, 1, 0, 1, 0, 1, 0, 1,
                         1, 0, 1, 1, 1, 0, 1,
                         1, 0, 0, 0, 0, 1}; //{341, 93, 33};
    int lenBarFactors[] = {9,7,6};
    int countBarFactors = 3;

    /*int *genCounts = malloc(decompCount*sizeof(int));*/

    /*unsigned long long pcn*/
        /*= processFFElements_useGens(xmipo, decompCount,*/
            /*polys, polysLen, polysCoeffDegs, polysCount, */
            /*evalToZero, mats, matLen, frobPowers,*/
            /*genCounts, m, charac, q,*/
            /*barFactors,lenBarFactors,countBarFactors,*/
            /*biggestPrimeFactor, lenBiggestPrimeFactor,*/
            /*matCharac, */
            /*0, 0, */
            /*multTable, initialMultShift, */
            /*addTable, initialAddShift);*/
    /*free(genCounts);*/
    /*printf("pcn=%i",pcn);*/
    ////////////////////////////////////////////////////////////
}

void test2(){
    // q = 2, n = 3
int polys[] = {
1, 0, 0, 
1, 0, 0, 
1, 0, 0, 
1, 0, 0, 
1, 0, 0, 
1, 0, 0, 
1, 0, 0
};
int mats[] = {
1, 0, 0, 
0, 1, 0, 
0, 0, 1, 
1, 0, 0, 
0, 0, 1, 
0, 1, 1, 
1, 0, 0, 
0, 1, 1, 
0, 1, 0
};
int matLen =  3 ;
int matCharac[] = {
1, 0, 0, 
0, 0, 1, 
0, 1, 1
};
int polysLen[] = {2, 1, 3, 1};
int polysCoeffDegs[] = {0, 0, 0, 0, 0, 0, 0};
bool evalToZero[] = {1, 0, 1, 0};
int frobPowers[] = {1, 1, 1, 1};
int polysCount[] = {2, 2};
int xmipo[] = {1, 1, 0, 1};
int barFactors[] = {1};
int lenBarFactors[] = {1};
int countBarFactors =  1 ;
int biggestPrimeFactor[] = {};
int lenBiggestPrimeFactor = 0;

int multTable[] = {1, 0, 1};
int initialMultShift = 1;
int addTable[] = {0, 1, 0, 1, 0};
int initialAddShift = 2;

int decompCount =  2 ;
int m =  3 ;
int charac =  2 ;
int q =  2 ;
    
    
    ////////////////////////////////////////////////////////////

    int *genCounts = malloc(decompCount*sizeof(int));

    unsigned long long pcn
        = processFFElements_useGens(xmipo, decompCount,
            polys, polysLen, polysCoeffDegs, polysCount, 
            evalToZero, mats, matLen, frobPowers,
            genCounts, m, charac, q,
            barFactors,lenBarFactors,countBarFactors,
            biggestPrimeFactor, lenBiggestPrimeFactor,
            matCharac, 
            0, 0, 
            multTable, initialMultShift, 
            addTable, initialAddShift);
    free(genCounts);
    printf("pcn=%i",pcn);

}


void mult_2_test(){
    int multTableRaw[] = {2, 0, 1, 2, 0, 1, 2, 0, 1};
    int initialMultShift = 4;
    int *multTable = multTableRaw+initialMultShift;
    int addTableRaw[] = {2, 0, 1, 2, 0, 1, 2, 0, 1};
    int initialAddShift = 4;
    int *addTable = addTableRaw+initialAddShift;

    int matCharac[] = 
        {1,2,1,
         0,1,1,
         0,0,1};
    int idcsMatCharac[] = 
        {0,1,2,
         1,2,
         2};
    int lenMatCharac[] = {3,2,1};

    int mipo[] = {1, 2, 0, 1};
    int idcsMipo[] = {3,1,0};
    int lenMipo = 3;


    int x[] = {0,2,0};
    int idcsX[] = {1};
    int lenX = 1;
    int x2[] = {1,0,0};
    int idcsX2[] = {0};
    int lenX2 = 1;

    int m = 3;
    int charac = 3;
    //////////////////////////////////////////////////////////////////////////

    int * tmp2 = malloc( 2*m*sizeof(int) );
    int * idcsTmp2 = malloc( 2*m*sizeof(int) );
    
    // Mult test /////////////////////////////////////////////////////////////
    /*int lenTmp2 = multiplyPolyShort_2(*/
            /*x,idcsX,lenX,*/
            /*x2,idcsX2,lenX2,*/
            /*tmp2,idcsTmp2,*/
            /*charac, multTable, addTable);*/
    /*printf("tmp2 = ");printArr(tmp2,2*m);*/
    /*printf("idcsTmp2=");printArr(idcsTmp2,lenTmp2);*/

    // Matmul test ///////////////////////////////////////////////////////////
    /*int lenTmp2 = matmulShort_2(*/
            /*matCharac,idcsMat,lenMat,*/
            /*x,idcsX,lenX,*/
            /*tmp2, idcsTmp2,*/
            /*m,charac,multTable,addTable);*/
    /*printf("tmp2=");printArr(tmp2,2*m);*/
    /*printf("idcsTmp2=");printArr(idcsTmp2,lenTmp2);*/
    
    // Modulo test ///////////////////////////////////////////////////////////
    /*int tmp2[] = {0,0,0, 0,0,1};*/
    /*int idcsTmp2[] = {5};*/
    /*int lenTmp2 = 1;*/
    /*lenTmp2 = moduloMonomShort_2(tmp2,idcsTmp2,lenTmp2,*/
            /*mipo,idcsMipo,lenMipo,*/
            /*charac,multTable,addTable);*/

    /*printf("tmp2=");printArr(tmp2,2*m);*/
    /*printf("idcsTmp2=");printArr(idcsTmp2,lenTmp2);*/
    
    // Power test ////////////////////////////////////////////////////////////
    /*int *ret = malloc(m*sizeof(int));*/
    /*int *idcsRet = malloc(m*sizeof(int));*/
    /*int power[] = {1,2,1}; //16*/
    /*int powerLen = 3;*/

    /*int lenRet = powerPolyShortCharac_2(*/
            /*x,idcsX,lenX,*/
            /*mipo,idcsMipo,lenMipo,*/
            /*ret, idcsRet,*/
            /*m,power,powerLen,*/
            /*matCharac,idcsMat,lenMat,*/
            /*charac,tmp2,multTable,addTable);*/

    /*printf("ret=");printArr(ret,m);*/
    /*printf("idcsRet=");printArr(idcsRet,lenRet);*/
    
    /*free(ret);*/
    /*free(idcsRet);*/
    // Power test ////////////////////////////////////////////////////////////
    int * tmpRet = malloc( m*sizeof(int) );
    int * idcsTmpRet = malloc( 2*m*sizeof(int) );
    int * tmp_x = malloc( m*sizeof(int) );
    int * idcsTmp_x = malloc( 2*m*sizeof(int) );
    int * tmp = malloc( m*sizeof(int) );
    int * idcsTmp = malloc( 2*m*sizeof(int) );
    int * tmpRet2 = malloc( m*sizeof(int) );
    int * idcsTmpRet2 = malloc( 2*m*sizeof(int) );
    
    int barFactors[] = {2,1};
    int lenBarFactors[] = {1,1};
    int biggestPrimeFactor[] = {1,1,1};
    int lenBiggestPrimeFactor = 3;
    int countBarFactors = 2;

    bool isPrim = isPrimitive_2(x,idcsX,lenX,
            mipo, idcsMipo,lenMipo,
            m, barFactors,lenBarFactors,countBarFactors,
            biggestPrimeFactor,lenBiggestPrimeFactor,
            matCharac,idcsMatCharac,lenMatCharac, charac,
            tmp_x, idcsTmp_x, tmp, idcsTmp,
            tmpRet,idcsTmpRet, tmpRet2,idcsTmpRet2, tmp2,
            multTable,addTable);
    if(isPrim)
        printf("isPrimitive!\n");
    else printf("is NOT primitive!\n");


    free(tmpRet);
    free(idcsTmpRet);
    free(tmp_x);
    free(idcsTmp_x);
    free(tmp);
    free(idcsTmp);
    free(tmpRet2);
    free(idcsTmpRet2);
    
    //////////////////////////////////////////////////////////////////////////
    free(idcsTmp2);
    free(tmp2);
}


void main(){
    mult_2_test();
}


void matmulTest(int *mipo, 
        int *polys, int *polysLen, int *polysCoeffDegs,
        int polysCount,
        int *mats, int matLen, int *frobPowers,
        int m, int charac,
        int *multTable, int initialMultShift,
        int *addTable, int initialAddShift){
    
    multTable += initialMultShift;
    addTable += initialAddShift;
    
    int * x = malloc( m*sizeof(int) );
    int * ret = malloc( m*sizeof(int) );
    int * tmp = malloc( m*sizeof(int) );
    int * tmp2 = malloc( 2*m*sizeof(int) );
    int * matmulCache = malloc(matLen*m*sizeof(int));
    bool * matmulCacheCalced = malloc(matLen*sizeof(bool));
    int i,j;

    initPoly(x,m);
    x[1] = 1;

    for(i=0;i<1000;i++){
        int curPolyPosition = 0;
        int curPolyCoeffPosition = 0;
        for(j=0;j<matLen;j++)
            matmulCacheCalced[j] = false;
        
        /*printf("testCN x=");printArr(x,m);*/
        for(j=0;j<polysCount;j++){
            /*printf("    apply poly=");printArr(polys+curPolyPosition,m*polysLen[j]);*/
            /*printf("           degs=");*/
                /*printArr(polysCoeffDegs+curPolyCoeffPosition,polysLen[j]);*/
            applyFrobShort(x, mipo, 
                    polys+curPolyPosition, polysLen[j],
                    polysCoeffDegs+curPolyCoeffPosition,
                    mats, frobPowers[j],
                    ret, m, charac, tmp, tmp2,
                    matmulCache, matmulCacheCalced,
                    multTable, addTable);
            
            curPolyPosition += m*polysLen[j];
            curPolyCoeffPosition += polysLen[j];
        }
    }
    free(x);
    free(ret);
    free(tmp);
    free(tmp2);
    free(matmulCache);
    free(matmulCacheCalced);
}

void primitiveTest(int *mipo, 
        int m, int charac,
        int *barFactors, int *lenBarFactors, int countBarFactors, 
        int *biggestPrimeFactor, int lenBiggestPrimeFactor,
        int *matCharac, 
        int *multTable, int initialMultShift,
        int *addTable, int initialAddShift){
    
    multTable += initialMultShift;
    addTable += initialAddShift;
    
    int * x = malloc( m*sizeof(int) );
    int * tmp = malloc( m*sizeof(int) );
    int * tmp_x = malloc( m*sizeof(int) );
    int * tmpRet = malloc( m*sizeof(int) );
    int * tmpRet2 = malloc( m*sizeof(int) );
    int * tmp2 = malloc( 2*m*sizeof(int) );
    int i,j;

    initPoly(x,m);
    x[1] = 1;
    for(j=0;j<10000;j++){
        isPrimitive(x,mipo,m,
                barFactors,lenBarFactors,countBarFactors,
                biggestPrimeFactor,lenBiggestPrimeFactor,
                matCharac,charac,
                tmp_x,tmp,tmpRet,tmpRet2,tmp2,
                multTable,addTable);
        x[0] += 1;
        if( x[0] == 2){
            for(i=0;i<m-1 && x[i]==2;i++){
                x[i] = 0;
                x[i+1] += 1;
            }
            if( x[m-1]==2){
                break;
            }
        }
        /*printf("is primitive!\n");*/
    /*else printf("is not primitive!\n");*/
    }

    free(x);
    free(tmp);
    free(tmp_x);
    free(tmpRet);
    free(tmpRet2);
    free(tmp2);
}

void primitiveTest_2(int *mipo, int *idcsMipo, int lenMipo,
        int m, int charac,
        int *barFactors, int *lenBarFactors, int countBarFactors, 
        int *biggestPrimeFactor, int lenBiggestPrimeFactor,
        int *matCharac, int *idcsMatCharac, int *lenMatCharac,
        int *multTable, int initialMultShift,
        int *addTable, int initialAddShift){
    
    multTable += initialMultShift;
    addTable += initialAddShift;
    
    int * x = malloc( m*sizeof(int) );
    int * idcsX = malloc( m*sizeof(int) );
    int * tmp = malloc( m*sizeof(int) );
    int * idcsTmp = malloc( 2*m*sizeof(int) );
    int * tmp_x = malloc( m*sizeof(int) );
    int * idcsTmp_x = malloc( 2*m*sizeof(int) );
    int * tmpRet = malloc( m*sizeof(int) );
    int * idcsTmpRet = malloc( 2*m*sizeof(int) );
    int * tmpRet2 = malloc( m*sizeof(int) );
    int * idcsTmpRet2 = malloc( 2*m*sizeof(int) );
    int * tmp2 = malloc( 2*m*sizeof(int) );
    int i,j;

    initPoly(x,m);
    initPoly(idcsX,m);
    x[1] = 1;
    idcsX[0] = 1;
    int lenX = 1;
    /*printf("test x=");printArr(x,m);*/
    /*printf("     idcsX=");printArr(idcsX,lenX);*/
    for(j=0;j<10000;j++){
        isPrimitive_2(x, idcsX, lenX,
                mipo, idcsMipo, lenMipo,
                m,
                barFactors,lenBarFactors,countBarFactors,
                biggestPrimeFactor,lenBiggestPrimeFactor,
                matCharac,idcsMatCharac, lenMatCharac,charac,
                tmp_x,idcsTmp_x,tmp,idcsTmp,tmpRet,idcsTmpRet,
                tmpRet2,idcsTmpRet2,tmp2,
                multTable,addTable);
        x[0] += 1;
        if( x[0] == 2){
            for(i=0;i<m-1 && x[i]==2;i++){
                x[i] = 0;
                x[i+1] += 1;
            }
            if( x[m-1]==2){
                break;
            }
        }
        /*printf("is primitive!\n");*/
    /*else printf("is not primitive!\n");*/
    }

    free(x);
    free(idcsX);
    free(tmp);
    free(idcsTmp);
    free(tmp_x);
    free(idcsTmp_x);
    free(tmpRet);
    free(idcsTmpRet);
    free(tmpRet2);
    free(idcsTmpRet2);
    free(tmp2);
}
