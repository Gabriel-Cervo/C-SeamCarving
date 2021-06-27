#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Para usar strings

#ifdef WIN32
#include <windows.h> // Apenas para Windows
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>   // Funções da OpenGL
#include <GL/glu.h>  // Funções da GLU
#include <GL/glut.h> // Funções da FreeGLUT
#endif

// SOIL é a biblioteca para leitura das imagens
#include <SOIL.h>

// Um pixel RGB (24 bits)
typedef struct
{
    unsigned char r, g, b;
} RGB8;

// Uma imagem RGB
typedef struct
{
    int width, height;
    RGB8 *img;
} Img;

// Protótipos
void load(char *name, Img *pic);
void uploadTexture();
void seamcarve(int targetWidth); // executa o algoritmo
void freemem();                  // limpa memória (caso tenha alocado dinamicamente)

// Funções da interface gráfica e OpenGL
void init();
void draw();
void keyboard(unsigned char key, int x, int y);
void arrow_keys(int a_keys, int x, int y);

// Funções próprias
void loadSourceEnergy(int rows, int columns, int matrix[rows][columns]);
void reduceEnergyInRedMask(int rows, int columns, int matrix[rows][columns]);
void loadAcumulatedEnergy(int rows, int columns, int matrix[rows][columns], int energiaSource[rows][columns]);
void findLowestSumPath(int rows, int columns, int outputArray[rows], int acumulatedSum[rows][columns]);
void applyResizing(int rows, int lowestAcumulatedSumPath[rows], int newW);

// Largura e altura da janela
int width, height;

// Largura desejada (selecionável)
int targetW;

int firstSeam = 1;

// Identificadores de textura
GLuint tex[3];

// As 3 imagens
Img pic[3];
Img *source;
Img *mask;
Img *target;

// Imagem selecionada (0,1,2)
int sel;

// Carrega uma imagem para a struct Img
void load(char *name, Img *pic)
{
    int chan;
    pic->img = (RGB8 *)SOIL_load_image(name, &pic->width, &pic->height, &chan, SOIL_LOAD_RGB);
    if (!pic->img)
    {
        printf("SOIL loading error: '%s'\n", SOIL_last_result());
        exit(1);
    }
    printf("Load: %d x %d x %d\n", pic->width, pic->height, chan);
}

//
// Implemente AQUI o seu algoritmo
void seamcarve(int targetWidth) {
    RGB8(*ptrSource)
    [source->width] = (RGB8(*)[source->width])source->img; // imagem de entrada

    RGB8(*ptrTarget)
    [target->width] = (RGB8(*)[target->width])target->img; // imagem de saida

    RGB8(*ptrMask)
    [mask->width] = (RGB8(*)[mask->width])mask->img; // imagem com mask
    
    // Copia imagem original na saida
    if (firstSeam == 1) {
        for (int y = 0; y < source->height; y++) {
            for (int x = 0; x < targetWidth; x++) {
                ptrTarget[y][x] = ptrSource[y][x];
            }
        }

        firstSeam = 0;
    }


    int energiaSource[target->height][targetWidth];
    int energiaSomada[target->height][targetWidth];
    int lowestAcumulatedSumPath[target->height];
    
    loadSourceEnergy(target->height, targetWidth, energiaSource);
    reduceEnergyInRedMask(target->height, targetWidth, energiaSource);
    loadAcumulatedEnergy(target->height, targetWidth, energiaSomada, energiaSource);
    findLowestSumPath(target->height, targetWidth, lowestAcumulatedSumPath, energiaSomada);
    applyResizing(target->height, lowestAcumulatedSumPath, targetWidth);

    // Deixa os pixels no width antigo em branco
    for (int y = 0; y < target->height; y++) {
        for (int x = targetWidth; x < target->width; x++) {
            ptrTarget[y][x].r = ptrTarget[y][x].g = ptrTarget[y][x].b = 255;
            ptrMask[y][x].r = ptrMask[y][x].g = ptrMask[y][x].b = 255;
        }
    }

    uploadTexture();   
    glutPostRedisplay();
}

void loadSourceEnergy(int rows, int columns, int matrix[rows][columns]) {
    RGB8(*ptrTarget)
    [target->width] = (RGB8(*)[target->width])target->img; // imagem de saida

    RGB8(*ptrMask)
    [mask->width] = (RGB8(*)[mask->width])mask->img; // imagem com mask

    int deltaRx, deltaGx, deltaBx;
    int deltaRy, deltaGy, deltaBy;

    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < columns; x++) {
            if (y == rows - 1) { // caso seja a ultima linha de pixels
                deltaRy = ptrTarget[y - 2][x].r - ptrTarget[y - 1][x].r;
                deltaGy = ptrTarget[y - 2][x].g - ptrTarget[y - 1][x].g;
                deltaBy = ptrTarget[y - 2][x].b - ptrTarget[y - 1][x].b;
            } else if (y == 0) { // Caso seja a primeira linha
                deltaRy = ptrTarget[y + 2][x].r - ptrTarget[y + 1][x].r;
                deltaGy = ptrTarget[y + 2][x].g - ptrTarget[y + 1][x].g;
                deltaBy = ptrTarget[y + 2][x].b - ptrTarget[y + 1][x].b;
            } else {
                deltaRy = ptrTarget[y + 1][x].r - ptrTarget[y - 1][x].r;
                deltaGy = ptrTarget[y + 1][x].g - ptrTarget[y - 1][x].g;
                deltaBy = ptrTarget[y + 1][x].b - ptrTarget[y - 1][x].b;
            }

            if (x == columns - 1) { // Caso Seja o ultimo pixel da direita
                deltaRx = ptrTarget[y][x - 2].r - ptrTarget[y][x - 1].r;
                deltaGx = ptrTarget[y][x - 2].g - ptrTarget[y][x - 1].g;
                deltaBx = ptrTarget[y][x - 2].b - ptrTarget[y][x - 1].b;
            } else if (x == 0) { // Caso seja o primeiro da esquerda
                deltaRx = ptrTarget[y][x + 2].r - ptrTarget[y][x + 1].r;
                deltaGx = ptrTarget[y][x + 2].g - ptrTarget[y][x + 1].g;
                deltaBx = ptrTarget[y][x + 2].b - ptrTarget[y][x + 1].b;
            } else { // Entre os dois
                deltaRx = ptrTarget[y][x + 1].r - ptrTarget[y][x - 1].r;
                deltaGx = ptrTarget[y][x + 1].g - ptrTarget[y][x - 1].g;
                deltaBx = ptrTarget[y][x + 1].b - ptrTarget[y][x - 1].b;
            }

            // Calculo do deltaX
            int deltaXFinal = (deltaRx * deltaRx) + (deltaGx * deltaGx) + (deltaBx * deltaBx);

            // Calculo do deltaY
            int deltaYFinal = (deltaRy * deltaRy) + (deltaGy * deltaGy) + (deltaBy * deltaBy);

            matrix[y][x] = deltaXFinal + deltaYFinal;
        }
    }
}

void loadAcumulatedEnergy(int rows, int columns, int matrix[rows][columns], int energiaSource[rows][columns]) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            if (i == 0) {
                matrix[i][j] = energiaSource[i][j];
            } else {
                int lowestValueInLine = matrix[i - 1][j];

                if (j < columns - 1) {
                    lowestValueInLine = lowestValueInLine > matrix[i - 1][j + 1] ? matrix[i - 1][j + 1] : lowestValueInLine;
                }

                if (j > 0) {
                    lowestValueInLine = lowestValueInLine > matrix[i - 1][j - 1] ? matrix[i - 1][j - 1] : lowestValueInLine;
                }

                matrix[i][j] = lowestValueInLine + energiaSource[i][j];
            }
        }
    }
}

void reduceEnergyInRedMask(int rows, int columns, int matrix[rows][columns]) {
     RGB8(*ptrMask)
     [mask->width] = (RGB8(*)[mask->width])mask->img; // imagem com mask

     for (int y = 0; y < rows; y++) {
         for (int x = 0; x < columns; x++) {
             if (ptrMask[y][x].r > 170) { // Area a remover
                if (ptrMask[y][x].g <= 30 && ptrMask[y][x].b <= 30) {
                    matrix[y][x] -= 799999;
                }
             } else if (ptrMask[y][x].g >= 170) { // Area a preservar
                if (ptrMask[y][x].r <= 30 && ptrMask[y][x].b <= 30) {
                    matrix[y][x] += 799999;
                }
             }
         }
     }
 }

void findLowestSumPath(int rows, int columns, int outputArray[rows], int acumulatedSum[rows][columns]) {
    int lowestAcumulatedSum = acumulatedSum[rows - 1][0];
    int startingIndex = 0;

    for (int j = 1; j < columns; j++) {
        if (lowestAcumulatedSum > acumulatedSum[rows - 1][j]) {
            lowestAcumulatedSum = acumulatedSum[rows - 1][j];
            startingIndex = j;
        }
    }

    int count = rows - 1;

    outputArray[count--] = startingIndex;
    int lowestIndex = startingIndex;
    int prevIndex = lowestIndex;

    for (int i = rows - 1; i > 0; i--) {
        int lowestValueOnTop = acumulatedSum[i - 1][prevIndex];

        if (prevIndex < columns - 1 && lowestValueOnTop > acumulatedSum[i - 1][prevIndex + 1]) {
            lowestValueOnTop = acumulatedSum[i - 1][prevIndex + 1];
            lowestIndex = prevIndex + 1;
        }

        if (prevIndex > 0 && lowestValueOnTop > acumulatedSum[i - 1][prevIndex - 1]) {
            lowestValueOnTop = acumulatedSum[i - 1][prevIndex - 1];
            lowestIndex = prevIndex - 1;
        }

        outputArray[count--] = lowestIndex;
        prevIndex = lowestIndex;
    }
}

void applyResizing(int rows, int lowestAcumulatedSumPath[rows], int newW) {
    RGB8(*ptrTarget)
    [target->width] = (RGB8(*)[target->width])target->img; // imagem de saida

    RGB8(*ptrMask)
    [mask->width] = (RGB8(*)[mask->width])mask->img; // imagem com mask

    // Percorre a imagem de saída preenchendo ela
    for (int y = 0; y < target->height; y++) {
        for (int x = lowestAcumulatedSumPath[y]; x < newW - 1; x++) {
            ptrTarget[y][x] = ptrTarget[y][x+1];
            ptrMask[y][x] = ptrMask[y][x+1];
        }
    }
}

void freemem()
{
    // Libera a memória ocupada pelas 3 imagens
    free(pic[0].img);
    free(pic[1].img);
    free(pic[2].img);
}

/********************************************************************
 * 
 *  VOCÊ NÃO DEVE ALTERAR NADA NO PROGRAMA A PARTIR DESTE PONTO!
 *
 ********************************************************************/
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("seamcarving [origem] [mascara]\n");
        printf("Origem é a imagem original, mascara é a máscara desejada\n");
        exit(1);
    }
    glutInit(&argc, argv);

    // Define do modo de operacao da GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // pic[0] -> imagem original
    // pic[1] -> máscara desejada
    // pic[2] -> resultado do algoritmo

    // Carrega as duas imagens
    load(argv[1], &pic[0]);
    load(argv[2], &pic[1]);

    if (pic[0].width != pic[1].width || pic[0].height != pic[1].height)
    {
        printf("Imagem e máscara com dimensões diferentes!\n");
        exit(1);
    }

    // A largura e altura da janela são calculadas de acordo com a maior
    // dimensão de cada imagem
    width = pic[0].width;
    height = pic[0].height;

    // A largura e altura da imagem de saída são iguais às da imagem original (1)
    pic[2].width = pic[1].width;
    pic[2].height = pic[1].height;

    // Ponteiros para as structs das imagens, para facilitar
    source = &pic[0];
    mask = &pic[1];
    target = &pic[2];

    // Largura desejada inicialmente é a largura da janela
    targetW = target->width;

    // Especifica o tamanho inicial em pixels da janela GLUT
    glutInitWindowSize(width, height);

    // Cria a janela passando como argumento o titulo da mesma
    glutCreateWindow("Seam Carving");

    // Registra a funcao callback de redesenho da janela de visualizacao
    glutDisplayFunc(draw);

    // Registra a funcao callback para tratamento das teclas ASCII
    glutKeyboardFunc(keyboard);

    // Registra a funcao callback para tratamento das setas
    glutSpecialFunc(arrow_keys);

    // Cria texturas em memória a partir dos pixels das imagens
    tex[0] = SOIL_create_OGL_texture((unsigned char *)pic[0].img, pic[0].width, pic[0].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
    tex[1] = SOIL_create_OGL_texture((unsigned char *)pic[1].img, pic[1].width, pic[1].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

    // Exibe as dimensões na tela, para conferência
    printf("Origem  : %s %d x %d\n", argv[1], pic[0].width, pic[0].height);
    printf("Máscara : %s %d x %d\n", argv[2], pic[1].width, pic[0].height);
    sel = 0; // pic1

    // Define a janela de visualizacao 2D
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0, width, height, 0.0);
    glMatrixMode(GL_MODELVIEW);

    // Aloca memória para a imagem de saída
    pic[2].img = malloc(pic[1].width * pic[1].height * 3); // W x H x 3 bytes (RGB)
    // Pinta a imagem resultante de preto!
    memset(pic[2].img, 0, width * height * 3);

    // Cria textura para a imagem de saída
    tex[2] = SOIL_create_OGL_texture((unsigned char *)pic[2].img, pic[2].width, pic[2].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

    // Entra no loop de eventos, não retorna
    glutMainLoop();
}

// Gerencia eventos de teclado
void keyboard(unsigned char key, int x, int y)
{
    if (key == 27)
    {
        // ESC: libera memória e finaliza
        freemem();
        exit(1);
    }
    if (key >= '1' && key <= '3')
        // 1-3: seleciona a imagem correspondente (origem, máscara e resultado)
        sel = key - '1';
    if (key == 's')
    {
        seamcarve(targetW);
    }
    glutPostRedisplay();
}

void arrow_keys(int a_keys, int x, int y)
{
    switch (a_keys)
    {
    case GLUT_KEY_RIGHT:
        if (targetW <= pic[2].width - 1)
            targetW += 1;
        seamcarve(targetW);
        break;
    case GLUT_KEY_LEFT:
        if (targetW > 1)
            targetW -= 1;
        seamcarve(targetW);
        break;
    default:
        break;
    }
}
// Faz upload da imagem para a textura,
// de forma a exibi-la na tela
void uploadTexture()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 target->width, target->height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, target->img);
    glDisable(GL_TEXTURE_2D);
}

// Callback de redesenho da tela
void draw()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Preto
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Para outras cores, veja exemplos em /etc/X11/rgb.txt

    glColor3ub(255, 255, 255); // branco

    // Ativa a textura corresponde à imagem desejada
    glBindTexture(GL_TEXTURE_2D, tex[sel]);
    // E desenha um retângulo que ocupa toda a tela
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);

    glTexCoord2f(0, 0);
    glVertex2f(0, 0);

    glTexCoord2f(1, 0);
    glVertex2f(pic[sel].width, 0);

    glTexCoord2f(1, 1);
    glVertex2f(pic[sel].width, pic[sel].height);

    glTexCoord2f(0, 1);
    glVertex2f(0, pic[sel].height);

    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Exibe a imagem
    glutSwapBuffers();
}
