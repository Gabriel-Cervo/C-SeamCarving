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
void loadAcumulatedEnergy(int rows, int columns, int matrix[rows][columns], int energiaSource[rows][columns]);
void reduceEnergyInRedMask(int rows, int columns, int matrix[rows][columns]);
void findLowestSumPath(int rows, int columns, int outputArray[rows], int acumulatedSum[rows][columns]);
void applyResizing(int rows, int lowestAcumulatedSumPath[rows]);

// Largura e altura da janela
int width, height;

// Largura desejada (selecionável)
int targetW;

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
    // Aplica o algoritmo e gera a saida em target->img...

    // Carrega a soma acumulada de energia
    int energiaSource[source->height][targetWidth];
    loadSourceEnergy(source->height, targetWidth, energiaSource);

    reduceEnergyInRedMask(source->height, targetWidth, energiaSource);

    int energiaSomada[source->height][targetWidth];
    loadAcumulatedEnergy(source->height, targetWidth, energiaSomada, energiaSource);

    printf("\n");
    for (int i = 0; i < source->height; i++) {
        for (int j = 0; j < targetWidth; j++) {
            printf("%8d, ", energiaSomada[i][j]);
        }
        printf("\n");
    }

    int lowestAcumulatedSumPath[source->height];
    findLowestSumPath(source->height, targetWidth, lowestAcumulatedSumPath, energiaSomada);

    applyResizing(source->height, lowestAcumulatedSumPath);

    uploadTexture();
    glutPostRedisplay();
}

void loadSourceEnergy(int rows, int columns, int matrix[rows][columns]) {
    RGB8(*ptrSource)
    [columns] = (RGB8(*)[columns])source->img; // imagem original

    int deltaRx, deltaGx, deltaBx;
    int deltaRy, deltaGy, deltaBy;

    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < columns; x++) {
            if (y == rows - 1) { // caso seja a ultima linha de pixels
                deltaRy = ptrSource[y - 2][x].r - ptrSource[y - 1][x].r;
                deltaGy = ptrSource[y - 2][x].g - ptrSource[y - 1][x].g;
                deltaBy = ptrSource[y - 2][x].b - ptrSource[y - 1][x].b;
            } else if (y == 0) { // Caso seja a primeira linha
                deltaRy = ptrSource[y + 2][x].r - ptrSource[y + 1][x].r;
                deltaGy = ptrSource[y + 2][x].g - ptrSource[y + 1][x].g;
                deltaBy = ptrSource[y + 2][x].b - ptrSource[y + 1][x].b;
            } else {
                deltaRy = ptrSource[y + 1][x].r - ptrSource[y - 1][x].r;
                deltaGy = ptrSource[y + 1][x].g - ptrSource[y - 1][x].g;
                deltaBy = ptrSource[y + 1][x].b - ptrSource[y - 1][x].b;
            }

            if (x == columns - 1) { // Caso Seja o ultimo pixel da direita
                deltaRx = ptrSource[y][x - 2].r - ptrSource[y][x - 1].r;
                deltaGx = ptrSource[y][x - 2].g - ptrSource[y][x - 1].g;
                deltaBx = ptrSource[y][x - 2].b - ptrSource[y][x - 1].b;
            } else if (x == 0) { // Caso seja o primeiro da esquerda
                deltaRx = ptrSource[y][x + 2].r - ptrSource[y][x + 1].r;
                deltaGx = ptrSource[y][x + 2].g - ptrSource[y][x + 1].g;
                deltaBx = ptrSource[y][x + 2].b - ptrSource[y][x + 1].b;
            } else { // Entre os dois
                deltaRx = ptrSource[y][x + 1].r - ptrSource[y][x - 1].r;
                deltaGx = ptrSource[y][x + 1].g - ptrSource[y][x - 1].g;
                deltaBx = ptrSource[y][x + 1].b - ptrSource[y][x - 1].b;
            }

            // Calculo do deltaX
            int deltaXFinal = (deltaRx * deltaRx) + (deltaGx * deltaGx) + (deltaBx * deltaBx);

            // Calculo do deltaY
            int deltaYFinal = (deltaRy * deltaRy) + (deltaGy * deltaGy) + (deltaBy * deltaBy);

            matrix[y][x] = deltaXFinal + deltaYFinal;
            printf("%8d, ", matrix[y][x]);
        }
        printf("\n");
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
            if (ptrMask[y][x].r > 240 && ptrMask[y][x].g < 100) { // Area a remover
                matrix[y][x] = -2147483;
            } else if (ptrMask[y][x].g > 240 && ptrMask[y][x].r < 100) { // Area a preservar
                matrix[y][x] = 2147483;
            }
        }
    }
}

void findLowestSumPath(int rows, int columns, int outputArray[rows], int acumulatedSum[rows][columns]) {
    printf("\n");
    
    int lowestAcumulatedSum = acumulatedSum[rows - 1][0];
    int startingIndex = 0;

    for (int j = 1; j < columns; j++) {
        if (lowestAcumulatedSum > acumulatedSum[rows - 1][j]) {
            lowestAcumulatedSum = acumulatedSum[rows - 1][j];
            startingIndex = j;
        }
    }

    int count = 0;

    outputArray[count++] = startingIndex;
    int lowestIndex = startingIndex;
    int prevIndex = lowestIndex;

    for (int i = rows - 2; i > 0; i--) {
        int lowestValueOnTop = acumulatedSum[i - 1][prevIndex];

        if (prevIndex < columns - 1 && lowestValueOnTop > acumulatedSum[i - 1][prevIndex + 1]) {
            lowestValueOnTop = acumulatedSum[i - 1][prevIndex + 1];
            lowestIndex = prevIndex + 1;
        }

        if (prevIndex > 0 && lowestValueOnTop > acumulatedSum[i - 1][prevIndex - 1]) {
            lowestValueOnTop = acumulatedSum[i - 1][prevIndex - 1];
            lowestIndex = prevIndex - 1;
        }

        outputArray[count++] = lowestIndex;
        prevIndex = lowestIndex;
        printf("%2d: %2d, ", prevIndex, lowestValueOnTop);
    }
}

void applyResizing(int rows, int lowestAcumulatedSumPath[rows]) {
    RGB8(*ptrSource)
    [source->width] = (RGB8(*)[source->width])source->img; // imagem original

    RGB8(*ptrTarget)
    [target->width] = (RGB8(*)[target->width])target->img; // imagem de saida

    int count = rows - 1;

    // Percorre a imagem de saída preenchendo ela
    for (int y = 0; y < target->height; y++) {
        // Preenche a imagem atual
        for (int x = 0; x < targetW; x++) {
            ptrTarget[y][x].r = ptrSource[y][x].r;
            ptrTarget[y][x].g = ptrSource[y][x].g;
            ptrTarget[y][x].b = ptrSource[y][x].b;
        }

        for (int x = lowestAcumulatedSumPath[count--]; x < target->width - 1; x++) {
            ptrTarget[y][x].r = ptrTarget[y][x+1].r;
            ptrTarget[y][x].g = ptrTarget[y][x+1].g;
            ptrTarget[y][x].b = ptrTarget[y][x+1].b;
        }

        // // Deixa os pixels no width antigo em preto
        // for (int x = targetW; x < target->width; x++) {
        //     ptrTarget[y][x].r = ptrTarget[y][x].g = ptrTarget[y][x].b = 0;
        // }
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
        if (targetW <= pic[2].width - 10)
            targetW += 10;
        seamcarve(targetW);
        break;
    case GLUT_KEY_LEFT:
        if (targetW > 10)
            targetW -= 10;
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
