#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>

// Estruturas de Dados
typedef struct {
    char nome[50];  // Nome do jogador
    int x, y;       // Posição do jogador no labirinto
    int vida;       // Pontos de vida do jogador
    int pontuacao;  // Pontuação do jogador
    int temChave;   // Indica se o jogador possui a chave
} Jogador;

typedef struct {
    int largura, altura;    // Dimensões do labirinto
    char **mapa;            // Matriz representando o labirinto
    int temChave;           // Indica se a fase possui uma chave
} Fase;

// Protótipo da função iniciarJogo (declarando antes de usar)
void iniciarJogo();

// Funções para Carregar e Liberar Fases
Fase* carregarFase(const char *nomeArquivo) {
    Fase *fase = (Fase*) malloc(sizeof(Fase));
    FILE *arquivo = fopen(nomeArquivo, "r");
    if (arquivo) {
        fscanf(arquivo, "%d %d", &fase->largura, &fase->altura);
        fase->mapa = (char**) malloc(fase->altura * sizeof(char*));
        fase->temChave = 0; // Inicializa sem chave
        
        for (int i = 0; i < fase->altura; i++) {
            fase->mapa[i] = (char*) malloc(fase->largura * sizeof(char));
            fscanf(arquivo, "%s", fase->mapa[i]);
            // Verifica se a fase contém uma chave
            if (strchr(fase->mapa[i], 'C')) {
                fase->temChave = 1;
            }
        }
        fclose(arquivo);
    }
    return fase;
}

void liberarFase(Fase *fase) {
    for (int i = 0; i < fase->altura; i++) {
        free(fase->mapa[i]);
    }
    free(fase->mapa);
    free(fase);
}

// Funções para Tela Principal e Estatísticas
void salvarEstatisticas(Jogador *jogador) {
    FILE *arquivo = fopen("estatisticas.bin", "ab");
    if (arquivo) {
        fwrite(jogador, sizeof(Jogador), 1, arquivo);
        fclose(arquivo);
    }
}

void carregarEstatisticas() {
    FILE *arquivo = fopen("estatisticas.bin", "rb");
    Jogador jogador;
    if (arquivo) {
        while (fread(&jogador, sizeof(Jogador), 1, arquivo)) {
            printf("Nome: %s | Pontuacao: %d | Vida: %d\n", jogador.nome, jogador.pontuacao, jogador.vida);
        }
        fclose(arquivo);
    } else {
        printf("Nenhuma estatistica encontrada.\n");
    }
}

void telaPrincipal() {
    int escolha;
    do {
        printf("1. Iniciar Jogo\n");
        printf("2. Estatisticas\n");
        printf("3. Sair\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &escolha);

        switch (escolha) {
            case 1:
                iniciarJogo();
                break;
            case 2:
                carregarEstatisticas();
                break;
            case 3:
                printf("Saindo do jogo.\n");
                break;
            default:
                printf("Opcao invalida.\n");
                break;
        }
    } while (escolha != 3);
}

// Função para Desenhar o Labirinto
void desenharLabirinto(Fase *fase, Jogador *jogador) {
    system("cls");
    printf("Jogador: %s | Pontos de Vida: %d | Pontuacao: %d | Chave: %s\n", 
           jogador->nome, jogador->vida, jogador->pontuacao, jogador->temChave ? "Sim" : "Nao");
    
    for (int y = 0; y < fase->altura; y++) {
        for (int x = 0; x < fase->largura; x++) {
            if (x == jogador->x && y == jogador->y) {
                printf("J ");
            } else {
                printf("%c ", fase->mapa[y][x]);
            }
        }
        printf("\n");
    }
    printf("\n");
}

// Função para Movimentação do Jogador
void moverJogador(Fase *fase, Jogador *jogador, char direcao) {
    int novaX = jogador->x;
    int novaY = jogador->y;

    if (direcao == 'W') novaY--;
    if (direcao == 'S') novaY++;
    if (direcao == 'A') novaX--;
    if (direcao == 'D') novaX++;

    if (novaX >= 0 && novaX < fase->largura && novaY >= 0 && novaY < fase->altura) {
        char celula = fase->mapa[novaY][novaX];
        if (celula == '.' || celula == 'S' || celula == 'C') {
            jogador->x = novaX;
            jogador->y = novaY;
            if (celula == 'S') {
                if (fase->temChave && !jogador->temChave) {
                    printf("Voce precisa da chave para sair!\n");
                } else {
                    printf("Parabens! Voce encontrou a saida!\n");
                    jogador->pontuacao += 100;
                }
            } else if (celula == 'C') {
                printf("Voce encontrou a chave!\n");
                jogador->temChave = 1;  // Coleta a chave
                fase->mapa[novaY][novaX] = '.';  // Remove a chave do mapa
            }
        } else if (celula == '#') {
            printf("Bateu em uma parede! Perdeu 10 pontos de vida.\n");
            jogador->vida -= 10;
        }
        desenharLabirinto(fase, jogador);
    } else {
        printf("Movimento invalido!\n");
    }
}

// Função Principal do Jogo
void iniciarJogo() {
    Jogador jogador = {.x = 1, .y = 1, .vida = 100, .pontuacao = 0, .temChave = 0};
    char faseArquivo[30];

    printf("Digite o nome do jogador: ");
    scanf(" %[^\n]s", jogador.nome);

    int jogadorSalvo = 0; // Variável para controlar se o jogador foi salvo

    for (int i = 1; i <= 5; i++) {
        snprintf(faseArquivo, sizeof(faseArquivo), "../fases/fase%d.txt", i);
        Fase *fase = carregarFase(faseArquivo);

        if (!fase) {
            printf("Erro ao carregar a fase %d\n", i);
            break;
        }

        printf("Fase %d carregada!\n", i);
        desenharLabirinto(fase, &jogador);

        char movimento;
        while (jogador.vida > 0) {
            printf("Digite W A S D para mover-se: ");
            scanf(" %c", &movimento);
            moverJogador(fase, &jogador, movimento);

            if (fase->mapa[jogador.y][jogador.x] == 'S' && (!fase->temChave || jogador.temChave)) {
                printf("Fase %d concluida!\n", i);
                break;
            }
        }

        liberarFase(fase);

        if (jogador.vida <= 0) {
            printf("Game Over! Voce perdeu todas as vidas.\n");
            salvarEstatisticas(&jogador);
            jogadorSalvo = 1; // Marca o jogador como salvo
            break;
        }

        jogador.temChave = 0;  // Reseta a chave para a próxima fase
    }

    // Salva as estatísticas apenas se o jogador não foi salvo anteriormente
    if (!jogadorSalvo) {
        salvarEstatisticas(&jogador);
    }
}

int main() {
    setlocale(LC_ALL, "pt_BR.UTF-8");
    telaPrincipal();
    return 0;
}
