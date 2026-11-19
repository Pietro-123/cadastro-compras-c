#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <ctype.h>
#define ID_PRODUTO 101
#define ID_QTD     102
#define ID_PRECO   103
#define ID_SALVAR  104
#define ID_MOSTRAR 105
#define ID_LIMPAR  106
#define ARQUIVO "aval1234.txt"

HWND janela, campoProduto, campoQtd, campoPreco, campoResultado, campoLista;

int fimValido(char *fim) {
    while (isspace((unsigned char)*fim)) fim++;
    return *fim == '\0';
}

int lerDecimal(HWND campo, double *valor) {
    char texto[100], *fim;
    GetWindowTextA(campo, texto, sizeof(texto));
    for (int i = 0; texto[i]; i++)
        if (texto[i] == ',') texto[i] = '.';
    errno = 0;
    *valor = strtod(texto, &fim);
    if (texto == fim || errno == ERANGE || !isfinite(*valor))
        return 0;
    return fimValido(fim);
}

int lerQuantidade(int *quantidade) {
    char texto[100], *fim;
    GetWindowTextA(campoQtd, texto, sizeof(texto));
    errno = 0;
    long numero = strtol(texto, &fim, 10);
    if (texto == fim || errno == ERANGE ||
        numero <= 0 || numero > 1000000)
        return 0;
    *quantidade = (int)numero;
    return fimValido(fim);
}

void salvarCompra() {
    char produto[100], mensagem[200];
    const char *falha = NULL;
    int quantidade;
    double preco, total;
    FILE *arquivo;
    GetWindowTextA(campoProduto, produto, sizeof(produto));
    if (strlen(produto) == 0 || strpbrk(produto, ";\n\r"))
        falha = "Informe um produto valido, sem ponto e virgula.";
    else if (!lerQuantidade(&quantidade))
        falha = "Digite uma quantidade inteira positiva!";
    else if (!lerDecimal(campoPreco, &preco) || preco < 0)
        falha = "Digite um preco valido!";
    else if (!isfinite(total = quantidade * preco))
        falha = "Total fora do intervalo permitido.";
    else if ((arquivo = fopen(ARQUIVO, "a")) == NULL)
        falha = "Nao foi possivel abrir o arquivo!";
    else {
        int ok = fprintf(arquivo, "%s;%d;%.2f;%.2f\n",
                         produto, quantidade, preco, total) >= 0 && !ferror(arquivo);
        if (fclose(arquivo) != 0 || !ok)
            falha = "Erro ao gravar a compra!";
    }
    if (falha) {
        MessageBoxA(NULL, falha, "Erro", MB_OK | MB_ICONERROR);
        return;
    }
    snprintf(mensagem, sizeof(mensagem), "Total da compra: R$ %.2f", total);
    SetWindowTextA(campoResultado, mensagem);
    MessageBoxA(NULL, "Compra cadastrada com sucesso!", "Sucesso", MB_OK | MB_ICONINFORMATION);
}

void mostrarCompras() {
    char linha[300], produto[100], texto[500];
    int quantidade, contador = 0;
    double preco, total, acumulado = 0;
    FILE *arquivo = fopen(ARQUIVO, "r");
    if (arquivo == NULL) {
        MessageBoxA(NULL, "Nenhuma compra cadastrada!", "Aviso", MB_OK | MB_ICONINFORMATION);
        return;
    }
    SendMessageA(campoLista, LB_RESETCONTENT, 0, 0);
    while (fgets(linha, sizeof(linha), arquivo))
        if (sscanf(linha, "%99[^;];%d;%lf;%lf", produto, &quantidade, &preco, &total) == 4) {
            snprintf(texto, sizeof(texto), "%s | Qtd: %d | Unit: %.2f | Total: %.2f",
                     produto, quantidade, preco, total);
            SendMessageA(campoLista, LB_ADDSTRING, 0, (LPARAM)texto);
            acumulado += total;
            contador++;
        }
    fclose(arquivo);
    snprintf(texto, sizeof(texto), "TOTAL GERAL: R$ %.2f | %d compras", acumulado, contador);
    SetWindowTextA(campoResultado, texto);
}

void limparCampos() {
    SetWindowTextA(campoProduto, "");
    SetWindowTextA(campoQtd, "");
    SetWindowTextA(campoPreco, "");
    SetWindowTextA(campoResultado, "Total: R$ 0,00");
    SendMessageA(campoLista, LB_RESETCONTENT, 0, 0);
}

HWND criar(const char *tipo, const char *texto, DWORD estilo,
           int x, int y, int largura, int altura, int id) {
    return CreateWindowA(tipo, texto, WS_VISIBLE | WS_CHILD | estilo,
                         x, y, largura, altura, janela, (HMENU)(INT_PTR)id, NULL, NULL);
}

LRESULT CALLBACK Janela(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        janela = hwnd;
        criar("STATIC", "Produto:", 0, 20, 20, 100, 25, 0);
        campoProduto = criar("EDIT", "", WS_BORDER | ES_AUTOHSCROLL, 130, 20, 300, 25, ID_PRODUTO);
        criar("STATIC", "Quantidade:", 0, 20, 60, 100, 25, 0);
        campoQtd = criar("EDIT", "", WS_BORDER | ES_AUTOHSCROLL, 130, 60, 120, 25, ID_QTD);
        criar("STATIC", "Preco unitario:", 0, 20, 100, 110, 25, 0);
        campoPreco = criar("EDIT", "", WS_BORDER | ES_AUTOHSCROLL, 130, 100, 120, 25, ID_PRECO);
        criar("BUTTON", "Salvar compra", 0, 20, 150, 130, 35, ID_SALVAR);
        criar("BUTTON", "Mostrar compras", 0, 160, 150, 140, 35, ID_MOSTRAR);
        criar("BUTTON", "Limpar", 0, 310, 150, 120, 35, ID_LIMPAR);
        campoResultado = criar("STATIC", "Total: R$ 0,00", 0, 20, 205, 430, 30, 0);
        criar("STATIC", "Compras cadastradas no arquivo:", 0, 20, 245, 400, 25, 0);
        campoLista = criar("LISTBOX", "", WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
                           20, 275, 430, 200, 0);
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
            case ID_SALVAR:  salvarCompra();   break;
            case ID_MOSTRAR: mostrarCompras(); break;
            case ID_LIMPAR:  limparCampos();   break;
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSA wc = {0};
    MSG msg;
    wc.lpfnWndProc = Janela;
    wc.hInstance = hInstance;
    wc.lpszClassName = "CadastroCompras";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    if (!RegisterClassA(&wc)) return 1;
    HWND hwnd = CreateWindowA("CadastroCompras", "Sistema de Cadastro de Compras em C",
                              WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 550,
                              NULL, NULL, hInstance, NULL);
    if (!hwnd) return 1;
    ShowWindow(hwnd, nCmdShow);
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}