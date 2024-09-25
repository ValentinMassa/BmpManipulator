#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "Funciones_Resano.h"
#include "TDA_VECTOR.h"



///
typedef void(*Acciones)(Pixeles*, const float, const int);
//para el Header
void EscribirHeaderEnImagNueva(HeaderBmp *, FILE*, unsigned char *, const int);

/////////////EFECTOS
void CambiarColorDeImagen(Pixeles**, FILE*, AdicDataBmp*,
                             int const, const float,
                                const int, const int, Acciones);
void Comodin(Pixeles**, FILE*, AdicDataBmp*,
                             int const, const float,
                                const int, const int, Acciones);
void cambiarTonalidad(Pixeles*, const float, const int);
void transformarAGris (Pixeles *, const float, const int);
void aumentarContraste (Pixeles * , const float , const int);
void reducirContraste (Pixeles *, const float, const int);
bool ActualizarDatosHeader(HeaderBmp*, AdicDataBmp *, const int, const int);
void ImpactarMatrizmagen(Pixeles **, const int, const int,unsigned char *, int, FILE*);
void ImpactarMatrizmagenRotarDerecha(Pixeles **, const int, const int,unsigned char *, int, FILE*);
void ImpactarMatrizmagenRotarIzquierda(Pixeles **, const int, const int,unsigned char *, int, FILE*);
void ImpactarMatrizHorizontal(Pixeles **, const int , const int , unsigned char * , int , FILE* );
void ImpactarMatrizVertical(Pixeles **, const int , const int , unsigned char * , int , FILE* );
void AchicarImagen(Pixeles **MatrizImagen, const int nuevoAlto, const int nuevoAncho,  const int originalAncho, const int originalAlto,
                          unsigned char * PaddingPunt, int padding, FILE* imagenNueva, int porcentaje);

/////////////////CODIGO

void cambiarTonalidad(Pixeles* pixel, const float procentaje, int const color)
{
    int valor;

    valor = (int)pixel->pixel[color] * procentaje;

    if(valor > 255)
        valor = 255;

    pixel->pixel[color] = (unsigned char)valor;
}

void transformarAGris (Pixeles *pixel,  const float No1, const int No2) // Este seria una copia. Para trabajar con el original (t_pixel *pixel)
{
    unsigned char promedio = (pixel->pixel[RED] + pixel->pixel[GREEN] + pixel->pixel[BLUE]) / 3;

    pixel->pixel[RED] = promedio;
    pixel->pixel[GREEN] = promedio;
    pixel->pixel[BLUE] = promedio;
}

void aumentarContraste (Pixeles *pixel , const float porcentaje, const int No2)
{
    int i,valorAumentado;

    for(i = 0 ; i < 3; i++)
    {
        valorAumentado = pixel->pixel[i] * porcentaje;
        if(valorAumentado > 255)
            valorAumentado = 255;

        pixel->pixel[i] = valorAumentado;
    }
}

void reducirContraste (Pixeles * pixel, const float porcentaje, const int No2)
{
    int i,valorDisminuido;

    for(i = 0 ; i < 3; i++)
    {
        valorDisminuido = pixel->pixel[i] * porcentaje;
        if(valorDisminuido < 0 )
            valorDisminuido = 0;

        pixel->pixel[i] = valorDisminuido;
    }
}

bool ActualizarDatosHeader(HeaderBmp* Header, AdicDataBmp * Data, const int alto, const int ancho)
{
    Header->palto = alto;
    Header->pancho = ancho;

    Data->padding = CalcularPadding(Header->pancho);
    if (!VerificarYGenerarVectorDeDatosPadding(Data))
    {
        puts("ERROR DE ASIGNACION DE MEMORIA EN FUNCION: ActualizarDatosHeader");
        return false;
    }
    //Recibimos una copia de header y una copia de RestoDataHeader, de lo cual usaremos
    //PaddingAdd(vector de n bytes de padding) y padding

    Header->tamImag = ((Header->palto) * (Header->pancho)) + (Data->padding * (Header->palto));
    Header->tamano = Header ->tamImag + Header->inicioDatos;
    printf("tamano: %d", ((Header->palto) * (Header->pancho)));

    printf("padding : %d", Data->padding);
    printf("inicio datos : %d", Header->inicioDatos);
    return true;
}

void CambiarColorDeImagen(Pixeles** Matriz, FILE* imagenFinal, AdicDataBmp* PaddingInfo,
                             int const color, const float porcentaje,
                                const int alto, const int ancho, Acciones accion)
{
    int i, j;
    Pixeles  aux;

    for(i=0; i<(alto); i++)
    {
        for(j=0;j<(ancho); j++)
        {
            aux = Matriz[i][j];
            accion(&aux,porcentaje,color);
            fwrite(&aux, sizeof(Pixeles), 1, imagenFinal);
        }
        if(PaddingInfo->padding!= 0)
            fwrite(PaddingInfo->PaddingAdd, PaddingInfo->padding, 1, imagenFinal);
    }
}

void Comodin(Pixeles** Matriz, FILE* imagenFinal, AdicDataBmp* PaddingInfo,
                             int const color, const float porcentaje,
                                const int alto, const int ancho, Acciones accion)
{
    int i, j, pixelesTotales;
    Pixeles  aux;

    pixelesTotales= ancho * alto;
    for(i=0; i<(alto); i++)
    {
        for(j=0;j<(ancho); j++)
        {
            aux = Matriz[i][j];
            if( j* i < (pixelesTotales/6))
                accion(&aux,20,BLUE);
            else if(j*i <(pixelesTotales/3))
                accion(&aux,20,RED);
            else accion(&aux,20,GREEN);
            fwrite(&aux, sizeof(Pixeles), 1, imagenFinal);
        }
        if(PaddingInfo->padding!= 0)
            fwrite(PaddingInfo->PaddingAdd, PaddingInfo->padding, 1, imagenFinal);
    }


}




bool imagenTransformada(VecEffectList * Datos, TDAVectList* vecImagen,
        Pixeles ** Matriz, AdicDataBmp * RestoDataImage ,
        HeaderBmp * Header , FILE* ImagenFinal)
{
    float valor =0;
    AdicDataBmp CopiaAdicData;
    HeaderBmp CopiaHeader;

     if (strcmp(Datos->NameEffect, "--comodin")==0)
    {
        rewind(ImagenFinal);
        EscribirHeaderEnImagNueva(Header, ImagenFinal, RestoDataImage->CabeceraDIBext, TamHeaderB);
        Comodin(Matriz, ImagenFinal, RestoDataImage, 0, 0,
        Header->palto, Header->pancho, cambiarTonalidad);
        return true;
    }

    if (strcmp(Datos->NameEffect, "--tonalidad-azul=")==0)
    {
        rewind(ImagenFinal);
        EscribirHeaderEnImagNueva(Header, ImagenFinal, RestoDataImage->CabeceraDIBext, TamHeaderB);
        valor = (float) (((float)(Datos->ProcentajeAAgregar)/(float)100) + (float)1) ;
        CambiarColorDeImagen(Matriz, ImagenFinal, RestoDataImage,
        BLUE, valor,
        Header->palto, Header->pancho, cambiarTonalidad);
        return true;
    }
    if (strcmp(Datos->NameEffect, "--tonalidad-verde=")==0)
    {
        rewind(ImagenFinal);
        EscribirHeaderEnImagNueva(Header, ImagenFinal, RestoDataImage->CabeceraDIBext, TamHeaderB);
        valor = (float) (((float)(Datos->ProcentajeAAgregar)/(float)100) + (float)1) ;
        CambiarColorDeImagen(Matriz, ImagenFinal, RestoDataImage,
        GREEN, valor,
        Header->palto, Header->pancho, cambiarTonalidad);

        return true;
    }
    if (strcmp(Datos->NameEffect, "--tonalidad-roja=")==0)
    {
        rewind(ImagenFinal);
        EscribirHeaderEnImagNueva(Header, ImagenFinal, RestoDataImage->CabeceraDIBext, TamHeaderB);
        valor = (float) (((float)(Datos->ProcentajeAAgregar)/(float)100) + (float)1) ;
        CambiarColorDeImagen(Matriz, ImagenFinal, RestoDataImage,
        RED, valor,
        Header->palto, Header->pancho, cambiarTonalidad);
        return true;
    }
    if (strcmp(Datos->NameEffect, "--escala-de-grises")==0)
    {
        rewind(ImagenFinal);
        EscribirHeaderEnImagNueva(Header, ImagenFinal, RestoDataImage->CabeceraDIBext, TamHeaderB);
        CambiarColorDeImagen(Matriz, ImagenFinal, RestoDataImage, 0, 0,
        Header->palto, Header->pancho, transformarAGris);
        return true;
    }
    if (strcmp(Datos->NameEffect, "--reducir-contraste=")==0)
    {
        rewind(ImagenFinal);
        EscribirHeaderEnImagNueva(Header, ImagenFinal, RestoDataImage->CabeceraDIBext, TamHeaderB);
        valor = (float)((float)(Datos->ProcentajeAAgregar)/(float)100);
        CambiarColorDeImagen(Matriz, ImagenFinal, RestoDataImage, 0,valor ,
        Header->palto, Header->pancho, reducirContraste);
        return true;
    }

    if (strcmp(Datos->NameEffect, "--aumentar-contraste=")==0) {
       rewind(ImagenFinal);
        EscribirHeaderEnImagNueva(Header, ImagenFinal, RestoDataImage->CabeceraDIBext, TamHeaderB);
        valor = (float) (((float)(Datos->ProcentajeAAgregar)/(float)100) + (float)1) ;
        CambiarColorDeImagen(Matriz, ImagenFinal, RestoDataImage, 0, valor,
        Header->palto, Header->pancho, aumentarContraste);
        return true;

    }

    if (strcmp(Datos->NameEffect, "--achicar=")==0) {
        CopiaHeader = *Header;
        CopiaAdicData.CabeceraDIBext = NULL;
        CopiaAdicData.PaddingAdd = NULL;
        CopiaAdicData.padding = 0;

        //CopiaAdicData->CabeceraDIBext esto no se usa, ya que copia el resto de datos 138-54 que
        //no difieren con la imagen original. siguen estando.

        //Copiamos las estructuras de datos ya que sino estariamos editando la iamgen original,
        //siendo que si el primer efecto es este, los siguientes trabajarian con una imagen recortada.
        if(!(ActualizarDatosHeader(&CopiaHeader, &CopiaAdicData,
                                         ( (int)( Header->palto * ((float)Datos->ProcentajeAAgregar/ ((float)100)  ))  ),
                                         ( (int)( Header->pancho * ((float)Datos->ProcentajeAAgregar)/ ((float)100)  )) )
             )
           )
        {
            puts("FUNCION ACHICAR TUVO UN ERROR EN MEMORIA, SE OMITIRA ESTA FUNCION");
            return true;
        }
        rewind(ImagenFinal);
        EscribirHeaderEnImagNueva(&CopiaHeader, ImagenFinal, RestoDataImage->CabeceraDIBext, TamHeaderB);
        AchicarImagen(Matriz, CopiaHeader.palto, CopiaHeader.pancho , Header->pancho, Header->palto, CopiaAdicData.PaddingAdd,
                            CopiaAdicData.padding, ImagenFinal,Datos->ProcentajeAAgregar );

        free(CopiaAdicData.PaddingAdd);  //liberamos el vector padding.
        //En caso que sea null (no hay padding) la buena practica de inicializarlo como null
        //le permite al free no fallar.
        return true;

    }

    if (strcmp(Datos->NameEffect, "--recortar=")==0)
        {

        CopiaHeader = *Header;
        CopiaAdicData.CabeceraDIBext = NULL;
        CopiaAdicData.PaddingAdd = NULL;
        CopiaAdicData.padding = 0;

        //CopiaAdicData->CabeceraDIBext esto no se usa, ya que copia el resto de datos 138-54 que
        //no difieren con la imagen original. siguen estando.

        //Copiamos las estructuras de datos ya que sino estariamos editando la iamgen original,
        //siendo que si el primer efecto es este, los siguientes trabajarian con una imagen recortada.
        if(!(ActualizarDatosHeader(&CopiaHeader, &CopiaAdicData,
                                         ( (int)( Header->palto * ( ((float)Datos->ProcentajeAAgregar/ ((float)100) ) ))  ),
                                         ( (int)( Header->pancho * ( ((float)Datos->ProcentajeAAgregar)/ ((float)100) ) )) )
             )
           )
        {
            puts("FUNCION RECORTAR TUVO UN ERROR EN MEMORIA, SE OMITIRA ESTA FUNCION");
            return true;
        }
        rewind(ImagenFinal);
        EscribirHeaderEnImagNueva(&CopiaHeader, ImagenFinal, RestoDataImage->CabeceraDIBext, TamHeaderB);
        ImpactarMatrizmagen(Matriz, CopiaHeader.palto, CopiaHeader.pancho , CopiaAdicData.PaddingAdd,
                            CopiaAdicData.padding, ImagenFinal);

        free(CopiaAdicData.PaddingAdd);  //liberamos el vector padding.
        //En caso que sea null (no hay padding) la buena practica de inicializarlo como null
        //le permite al free no fallar.
        return true;
    }

    if (strcmp(Datos->NameEffect, "--rotar-derecha")==0 || strcmp(Datos->NameEffect, "--rotar-izquierda")==0)
    {
        CopiaHeader = *Header;
        CopiaAdicData.CabeceraDIBext = NULL;
        CopiaAdicData.PaddingAdd = NULL;
        CopiaAdicData.padding = 0;

        if(!(ActualizarDatosHeader(&CopiaHeader, &CopiaAdicData, Header->pancho, Header->palto)))
        {
            puts("FUNCION ROTAR TUVO UN ERROR EN MEMORIA, SE OMITIRA ESTA FUNCION");
            return true;
        }
        rewind(ImagenFinal);
        EscribirHeaderEnImagNueva(&CopiaHeader, ImagenFinal, RestoDataImage->CabeceraDIBext, TamHeaderB);

        if(strcmp(Datos->NameEffect, "--rotar-derecha")==0)
        {
            ImpactarMatrizmagenRotarDerecha(Matriz, CopiaHeader.palto, CopiaHeader.pancho, CopiaAdicData.PaddingAdd,
                            CopiaAdicData.padding, ImagenFinal);
        }
        else
        {
            ImpactarMatrizmagenRotarIzquierda(Matriz, CopiaHeader.palto, CopiaHeader.pancho, CopiaAdicData.PaddingAdd,
                            CopiaAdicData.padding, ImagenFinal);
        }

        free(CopiaAdicData.PaddingAdd);
        return true;
    }


    if ( (strcmp(Datos->NameEffect, "--espejar-horizontal")==0) || (strcmp(Datos->NameEffect, "--espejar-vertical") ==0 ) )
    {
        rewind(ImagenFinal);
        EscribirHeaderEnImagNueva(Header, ImagenFinal, RestoDataImage->CabeceraDIBext, TamHeaderB);

        if(strcmp(Datos->NameEffect, "--espejar-vertical")==0)
        {
                ImpactarMatrizVertical(Matriz, Header->palto, Header->pancho, RestoDataImage->PaddingAdd,
                            RestoDataImage->padding, ImagenFinal);
        }
        else
        {
            ImpactarMatrizHorizontal(Matriz, Header->palto, Header->pancho, RestoDataImage->PaddingAdd,
                            RestoDataImage->padding, ImagenFinal);
        }
        return true;

    }

    return true;
}

void AchicarImagen(Pixeles **MatrizImagen, const int nuevoAlto, const int nuevoAncho, const int originalAncho, const int originalAlto,
                          unsigned char * PaddingPunt, int padding, FILE* imagenNueva, int porcentaje)
{
    int i, j;
    float escala= porcentaje/100.0f;
    // Asignaci�n de memoria para new_image
    Pixeles **new_image = malloc(nuevoAlto * sizeof(Pixeles *));
    for (i = 0; i < nuevoAlto; i++) {
        new_image[i] = malloc(nuevoAncho * sizeof(Pixeles));
    }


    for (i = 0; i < nuevoAlto; i++) {
        for (j = 0; j < nuevoAncho; j++) {
            int original_x = (int)(j / escala);  // Cambi� i por j
            int original_y = (int)(i / escala);  // Cambi� i por j
            if (original_x < originalAncho && original_y < originalAlto) {
                new_image[i][j] = MatrizImagen[original_y][original_x];
            }
            fwrite(&new_image[i][j], sizeof(Pixeles), 1, imagenNueva);
        }

        if (padding != 0)
            fwrite(PaddingPunt, padding, 1, imagenNueva);
    }
    // Liberar memoria
    for (i = 0; i < nuevoAlto; i++) {
        free(new_image[i]);
    }
    free(new_image);

}
void ImpactarMatrizmagen(Pixeles **MatrizImagen, const int alto, const int ancho,
                          unsigned char * PaddingPunt, int padding, FILE* imagenNueva)
{
    int i, j;

    for(i=0; i<(alto); i++)
    {
        for(j=0;j<(ancho); j++)
        {
            fwrite(&MatrizImagen[i][j], sizeof(Pixeles), 1, imagenNueva);
        }

        if(padding!= 0)
            fwrite(PaddingPunt, padding, 1, imagenNueva);
    }

}
void ImpactarMatrizVertical(Pixeles **MatrizImagen, const int alto, const int ancho,
                          unsigned char * PaddingPunt, int padding, FILE* imagenNueva)
{
    int i, j;

    for(i= alto -1 ; i >= 0; i--)
    {
        for(j= 0; j < ancho; j++)
        {
            fwrite(&MatrizImagen[i][j], sizeof(Pixeles), 1, imagenNueva);
        }

        if(padding!= 0)
            fwrite(PaddingPunt, padding, 1, imagenNueva);
    }

}

void ImpactarMatrizHorizontal(Pixeles **MatrizImagen, const int alto, const int ancho,
                          unsigned char * PaddingPunt, int padding, FILE* imagenNueva)
{
    int i, j;

    for(i= 0; i < alto; i++)
    {
        for(j= ancho -1; j >= 0; j--)
        {
            fwrite(&MatrizImagen[i][j], sizeof(Pixeles), 1, imagenNueva);
        }

        if(padding!= 0)
            fwrite(PaddingPunt, padding, 1, imagenNueva);
    }

}

void ImpactarMatrizmagenRotarDerecha(Pixeles **MatrizImagen, const int alto, const int ancho,
                          unsigned char * PaddingPunt, int padding, FILE* imagenNueva)
{
    int i, j;

    for(i=0; i<(alto); i++)
    {
        for(j=0;j<(ancho); j++)
        {
            fwrite(&MatrizImagen[j][i], sizeof(Pixeles), 1, imagenNueva);
        }

        if(padding!= 0)
            fwrite(PaddingPunt, padding, 1, imagenNueva);
    }

}

void ImpactarMatrizmagenRotarIzquierda(Pixeles **MatrizImagen, const int alto, const int ancho,
                          unsigned char * PaddingPunt, int padding, FILE* imagenNueva)
{
    int i, j;

    for(i = 0; i < alto; i++)
    {
        for(j= (ancho -1); j>=(0); j--)
        {
            fwrite(&MatrizImagen[j][i], sizeof(Pixeles), 1, imagenNueva);
        }

        if(padding!= 0)
            fwrite(PaddingPunt, padding, 1, imagenNueva);
    }

}



void EscribirHeaderEnImagNueva(HeaderBmp * Header, FILE* imagen,
                                unsigned char * restoHeader, const int tamHeader)
{
    fwrite(Header->bm, sizeof(Header->bm), 1, imagen);
    fwrite(&Header->tamano,sizeof(Header->tamano),1, imagen);
    fwrite(&Header->reservado1,sizeof(Header->reservado1),1, imagen);
    fwrite(&Header->reservado2,sizeof(Header->reservado2),1, imagen);
    fwrite(&Header->inicioDatos,sizeof(Header->inicioDatos),1 , imagen);
    fwrite(&Header->TamCabecera,sizeof(Header->TamCabecera),1, imagen);
    fwrite(&Header->pancho,sizeof(Header->pancho),1, imagen);
    fwrite(&Header->palto,sizeof(Header->palto),1, imagen);
    fwrite(&Header->numeroPlanos,sizeof(Header->numeroPlanos),1, imagen);
    fwrite(&Header->tamPuntos,sizeof(Header->tamPuntos),1, imagen);
    fwrite(&Header->tipoCompresion,sizeof(Header->tipoCompresion),1, imagen);
    fwrite(&Header->tamImag,sizeof(Header->tamImag),1, imagen);
    fwrite(&Header->pxmh,sizeof(Header->pxmh),1, imagen);
    fwrite(&Header->pxmv,sizeof(Header->pxmv),1, imagen);
    fwrite(&Header->coloresUsados, sizeof(Header->coloresUsados),1, imagen);
    fwrite(&Header->coloresImportantes,sizeof(Header->coloresImportantes),1, imagen);

    if(Header->inicioDatos > tamHeader) //SIGNIFICA QUE SI ES 138 EL INICIO Y EL TAM ES 54, hay datos adicionales
    {
        fwrite(restoHeader, Header->inicioDatos - tamHeader,1, imagen);
    }
}



