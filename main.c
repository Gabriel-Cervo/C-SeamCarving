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
    
    RGB8(*ptrMask)
    [mask->width] = (RGB8(*)[mask->width])mask->img; // imagem com mask
    
    RGB8(*ptrTarget)
    [target->width] = (RGB8(*)[target->width])target->img; // imagem de saida

    // Carrega a soma acumulada de energia
    int energiaSource[source->height][source->width];
    loadSourceEnergy(source->height, source->width, energiaSource);

    int energiaSomada[source->height][source->width]; 
    loadAcumulatedEnergy(source->height, source->width, energiaSomada, energiaSource);

    // Percorre a imagem de saída preenchendo ela
    for (int y = 0; y < target->height; y++) {
        // Preenche pixels no novo width
        for (int x = 0; x < targetW; x++) {
            ptrTarget[y][x].r = ptrTarget[y][x].g = 255;
        }

        // Deixa os pixels no width antigo em preto
        for (int x = targetW; x < target->width; x++) {
            ptrTarget[y][x].r = ptrTarget[y][x].g = 0;
        }
    }

    // // O que fazer com isso?
    // // Imagem original selecionada
    // if (sel == 0) {

    // }

    // // Imagem com mask selecionada
    // if (sel == 1) {

    // }

    // // Imagem de saída selecionada
    // if (sel == 2) {

    // }

    // Chame uploadTexture a cada vez que mudar
    // a imagem (pic[2])
    uploadTexture();
    glutPostRedisplay();
}

void loadSourceEnergy(int rows, int columns, int matrix[rows][columns]) {
     RGB8(*ptrSource)
    [columns] = (RGB8(*)[columns])source->img; // imagem original

        // Calculo de energia
    for (int y = 0; y < rows; y++) {
            // Pega os indices a serem usados no calculo delta Y
            // Só é necessário pegar uma vez a cada pixel de altura, pois todos na mesma linha utilizam ele
            int indexY1 = y + 1;
            int indexY2 = y - 1;

            // Pega de volta para o topo caso extrapole
            if (indexY1 > rows) {
                indexY1 = 0;
            }

            // Pega o último caso extrapole
            if (indexY2 < 0) {
                indexY2 = rows;
            }

        for (int x = 0; x < columns; x++) {
            // Pega os indices a serem usados no calculo delta X
            int index1 = x + 1;
            int index2 = x - 1;
        
            // Pega o primeiro da esquerda caso extrapole
            if (index1 > columns) {
                index1 = 0;
            }

            // Pega o último da direita caso extrapole
            if (index2 < 0) {
                index2 = columns;
            }

            // Calculo do deltaX
            int deltaRx = ptrSource[y][index1].r - ptrSource[y][index2].r;
            int deltaGx = ptrSource[y][index1].g - ptrSource[y][index2].g;
            int deltaBx = ptrSource[y][index1].b - ptrSource[y][index2].b;
            int deltaXFinal = (deltaRx * deltaRx) + (deltaGx * deltaGx) + (deltaBx * deltaBx);

            // Calculo do deltaY 
            deltaRx = ptrSource[indexY1][x].r - ptrSource[indexY2][x].r;
            deltaGx = ptrSource[indexY1][x].g - ptrSource[indexY2][x].g;
            deltaBx = ptrSource[indexY1][x].b - ptrSource[indexY2][x].b;
            int deltaYFinal = (deltaRx * deltaRx) + (deltaGx * deltaGx) + (deltaBx * deltaBx);

            matrix[y][x] = deltaXFinal + deltaYFinal;
        }
    }
}

void loadAcumulatedEnergy(int rows, int columns, int matrix[rows][columns], int energiaSource[rows][columns]) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            if (i == 0) {
                matrix[i][j] = matrix[i][j];
                break;
            } else if (i == (rows - 1)) {
                break;
            }

            // Soma da diagonal da esquerda
            if (j > 1) {
                if (energiaSource[i][j] < energiaSource[i][j - 1] && (energiaSource[i][j] < energiaSource[i][j - 2])) {
                    matrix[i + 1][j - 1] = energiaSource[i + 1][j - 1] + energiaSource[i][j];
                }

            } else if(j == 1){ 
                if (energiaSource[i][j] < energiaSource[i][j - 1]) {
                        matrix[i + 1][j - 1] = energiaSource[i + 1][j - 1] + energiaSource[i][j];
                    }
            } 

            // Soma do valor de baixo
            if (j == 0){
                if((energiaSource[i][j] < energiaSource[i][j + 1])) {
                    matrix[i + 1][j] = energiaSource[i + 1][j] + energiaSource[i][j];
                }

            } else if (j > 0 && j < columns - 1) {
                if (energiaSource[i][j] < energiaSource[i][j - 1] && (energiaSource[i][j] < energiaSource[i][j + 1])) {
                    matrix[i + 1][j] = energiaSource[i + 1][j] + energiaSource[i][j];
                }

            } else if (j == columns - 1) {
                if((energiaSource[i][j] < energiaSource[i][j - 1])){
                    matrix[i + 1][j] = energiaSource[i + 1][j] + energiaSource[i][j];
                }

            }

            // Soma da diagonal da direita
            if (j < columns - 2) {
                if (energiaSource[i][j] < energiaSource[i][j + 1] && (energiaSource[i][j] < energiaSource[i][j + 2])) {
                    matrix[i + 1][j + 1] = energiaSource[i + 1][j + 1] + energiaSource[i][j];
                }

            } else if(j == columns - 2) { 
                if (energiaSource[i][j] < energiaSource[i][j + 1]){
                        matrix[i + 1][j + 1] = energiaSource[i + 1][j + 1] + energiaSource[i][j];
                    }
            }                  
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
