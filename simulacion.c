/*
Javier Alejandro Acevedo Barroso
Primer Bosquejo. 1D con método de fourier.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <fftw3.h>

//Constantes de la simulación.
#define PI 3.14159265359

//Valores límites para la posición.
#define Xmin -5
#define Xmax 5

//Valores límites para la velocidad
#define Vmin -1
#define Vmax 1

#define Nx 512
#define Nv 512


//Arreglos
double phase[Nx][Nv];
double *density;
double *acce;

//Variables
int i;
int j;
int k;

double dx = (Xmax-Xmin)*1.0/Nx;
double dv = (Vmax-Vmin)*1.0/Nv;

FILE *constantes;

//Métodos
void printPhase(char *name);
double gaussD(double x, double v, double sx, double sv, double amplitude);
double calDensity();
void printDensity(char *name);
void printConstant(char *name, double value);
double giveDensity(int l);
double vFourier();
double calcK2(double j2);


int main()
{
    density = malloc((sizeof(double)*Nx));
	constantes = fopen("constants.dat","w+");
	printConstant("Xmin",Xmin);
	printConstant("Xmax",Xmax);
	printConstant("Vmin",Vmin);
	printConstant("Vmax",Vmax);
	printConstant("Nx",Nx);
	printConstant("Nv",Nv);
	double x;
	double v;
	for(i=0;i<Nx;i+=1) {
		for(j=0;j<Nv;j+=1){
			x = Xmin*1.0+dx*i;
			v = Vmin*1.0+dv*j;
			phase[i][j] = gaussD(x,v,5,0.1, 0.1);
				}
			}
	printPhase("grid.dat");
	double mass = calDensity();
	printf("%f\n",mass);
	printDensity("density.dat");

    vFourier();

// TODO (clarko#1#): Añadir método para calcular la aceleración.


	fclose(constantes);
	return 0;

}



void printPhase(char *name)
{
	FILE *output = fopen(name, "w+");
	for(i=0;i<Nx;i+=1) {
		for(j=0;j<Nv;j+=1){
			fprintf(output,"%f ", phase[i][j]);
				}
		fprintf(output,"\n");
			}
	fclose(output);

}

void printDensity(char *name)
{
	FILE *output = fopen(name, "w+");
	for(i=0;i<Nx;i+=1) {
            fprintf(output, "%f\n",density[i]);
			}
	fclose(output);
}

void printConstant(char *name, double value)
{
    fprintf(constantes, "%s", name);
    fprintf(constantes, " %f\n", value);

}


double gaussD(double x, double v, double sx, double sv, double amplitude)
{
	double ex = -x*x/(2.0*sx*sx)-v*v/(2.0*sv*sv);
	return amplitude*exp(ex);

}

double calDensity()
{
	double mass = 0;
	for(i = 0;i<Nx;i+=1){
		density[i]=0;
		for(j=0;j<Nv;j+=1){
				density[i] += phase[i][j]*dv;
			}
		mass += density[i];

		}
	return mass;
}

double vFourier()
{
//    double * densityIN= malloc(sizeof(double)*Nx);
//    for(i = 0;i<Nx;i+=1){
//        densityIN[i] = giveDensity(i);
//    }


    fftw_complex *in, *out, *inR, *mem;
    in=(fftw_complex*) fftw_malloc(sizeof(fftw_complex)*Nx);
    out=(fftw_complex*) fftw_malloc(sizeof(fftw_complex)*Nx);
    inR=(fftw_complex*) fftw_malloc(sizeof(fftw_complex)*Nx);
    mem=(fftw_complex*) fftw_malloc(sizeof(fftw_complex)*Nx);

    fftw_plan pIda, pVuelta;
    pIda = fftw_plan_dft_1d(Nx, in, out,FFTW_FORWARD, FFTW_MEASURE);

    FILE *input = fopen("inF.dat", "w+");
    FILE *output0 = fopen("outF0.dat", "w+");
    FILE *output1 = fopen("outF1.dat", "w+");
    FILE *oR = fopen("oR.dat", "w+");
    FILE *oI = fopen("oI.dat", "w+");


    //Cargar densidad en in:
    for(i=0;i<Nx;i+=1){
        //in[0][i] = densityIN[i];
        in[i] = giveDensity(i);
        inR[i] = -1.0;
        //in[i] = sin(2.0*PI*i*deltax);//Actualmente funciona para sin(x) confirmado. (se esperaba que la parte real de out fuera 0 y la imaginaria tuviera los picos, sin embargo solo el módulo cumple esto)
        //printf("%f, %d,%d\n", in[0][i], i,Nx);
        fprintf(input, "%f\n",creal(in[i]));
    }
//    printf("Press enter to continue...\n");
//    getchar();


    fftw_execute(pIda);

    //Guarda out en mem. Imprime a archivo.
    for(i=0;i<Nx;i+=1){
        mem[i] = out[i];
        fprintf(output0, "%f\n",creal(out[i]));
        fprintf(output1, "%f\n",cimag(out[i]));
    }

    pIda = fftw_plan_dft_1d(Nx, out, inR,
    FFTW_BACKWARD, FFTW_MEASURE); //Se debe usar el mismo plan sí o sí al parecer.
    double memDo;

    //Devuelve carga a out Î(Chi).
    for(i=1;i<Nx;i+=1){
        out[i] = -mem[i]/calcK2((double)i);
        memDo = out[i];//Evita problemas de casting.
        printf("%f %f %d \n",memDo, calcK2((double) i), i);
    }



    fftw_execute(pIda);


    for(i=0;i<Nx;i+=1){
        fprintf(oR, "%f\n",creal(inR[i])/Nx);
        fprintf(oI, "%f\n",cimag(inR[i])/Nx);
    }

    fclose(input);
    fclose(output0);
    fclose(output1);
    fclose(oR);
    fclose(oI);





}

//Calcula el k**2 de mis notas.
double calcK2(double j2)
{
    if(j2 == Nx/2.0){
        return 1.0;
    }
    double k2= 2.0*sin(dx*PI*j2)/dx;
    return pow(k2,2);
}

//Método para evitar efectos misticos relacionado a pointers.
double giveDensity(int l)
{
    double rta = density[l];
    return rta;
}






















