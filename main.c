#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "aes.h"

GtkBuilder *builder;

static void Encrypt_Main(char *path, char *key);
static void Decrypt_Main(char *path, char *key);
static unsigned char *LerArquivo(char *path, int *size);
static void *CriarArquivo(unsigned char *output, int size);

//Printa a string como um hex
static void phex(unsigned char* str)
{
    unsigned char len = 16;

    unsigned char i;
    for (i = 0; i < len; i++)
        printf("%02x", str[i]);
    printf("\n");
}

//Funções relacionadas a interface gráfica
void onDestroy(GtkWidget *widget, gpointer data){
    gtk_main_quit();
}
void btn_enc_clicked(GtkWidget *widget, gpointer data){
    GtkWidget *encWindow = GTK_WIDGET(gtk_builder_get_object(builder, "encWindow"));
    gtk_window_set_title(GTK_WINDOW(encWindow), "Encriptar");
    gtk_widget_show_all(encWindow);
}
void btn_dec_clicked(GtkWidget *widget, gpointer data){
    GtkWidget *decWindow = GTK_WIDGET(gtk_builder_get_object(builder, "decWindow"));
    gtk_window_set_title(GTK_WINDOW(decWindow), "Descriptografar");
    gtk_widget_show_all(decWindow);
}
void enc_Clicked(GtkWidget *widget, gpointer data){
    GtkEntry *keyWidget = GTK_ENTRY(gtk_builder_get_object(builder, "encKey"));
    GtkFileChooserButton *fileWidget = GTK_FILE_CHOOSER_BUTTON(gtk_builder_get_object(builder, "encFile"));

    //Checar se a chave é invalida
    if(strlen(gtk_entry_get_text(keyWidget)) != 16){
        GtkDialog *dialog = GTK_DIALOG(gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "VALOR INVALIDO!"));
        gtk_dialog_run(dialog);
        gtk_widget_destroy(GTK_WIDGET(dialog));
    }
    else
    {
        Encrypt_Main((char*)gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fileWidget)), (char*)gtk_entry_get_text(keyWidget));
    }
}
void dec_Clicked(GtkWidget *widget, gpointer data){
    GtkEntry *keyWidget = GTK_ENTRY(gtk_builder_get_object(builder, "decKey"));
    GtkFileChooserButton *fileWidget = GTK_FILE_CHOOSER_BUTTON(gtk_builder_get_object(builder, "decFile"));

    //Checar se a chave é invalida
    if(strlen(gtk_entry_get_text(keyWidget)) != 16){
        GtkDialog *dialog = GTK_DIALOG(gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "VALOR INVALIDO!"));
        gtk_dialog_run(dialog);
        gtk_widget_destroy(GTK_WIDGET(dialog));
    }
    else
    {
        Decrypt_Main((char*)gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fileWidget)), (char*)gtk_entry_get_text(keyWidget));
    }
}
gboolean enc_Close(GtkWidget *widget, gpointer data){
    gtk_widget_hide(widget);
    return 1;
}

int main (int argc, char *argv[])
{
    //Iniciar e setar o GTK
    gtk_init(&argc, &argv);
    builder = gtk_builder_new_from_file("ui.glade");
    gtk_builder_add_callback_symbols(builder,
    "onDestroy", G_CALLBACK(onDestroy),
    "btn_enc_clicked", G_CALLBACK(btn_enc_clicked),
    "btn_dec_clicked", G_CALLBACK(btn_dec_clicked),
    "enc_Clicked", G_CALLBACK(enc_Clicked),
    "dec_Clicked", G_CALLBACK(dec_Clicked),
    "enc_Close", G_CALLBACK(enc_Close), NULL);
    gtk_builder_connect_signals(builder, NULL);
    GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "mainWindow"));
    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}

static void Encrypt_Main(char *path, char *key)
{
    unsigned char i;
    int tmp;
    int size;
    //Fazer fill caso não multiplo de 16
    char* texto = LerArquivo(path, &tmp);
    if (strlen(texto) % 16) {
        printf("Texto nao multiplo de 16, executando filling...\n");
        int tamanhoTexto = tmp;
        char* buff = (char*)malloc(tamanhoTexto);
        memcpy(buff, texto, tamanhoTexto);
        int newSize = strlen(texto) + (16 - (tamanhoTexto % 16));
        texto = malloc(newSize*sizeof(char));
        memcpy(texto, buff, tamanhoTexto);
        printf("OLD SIZE: %d\nNEW SIZE: %d\n", tmp, newSize);
        size = newSize;
        for (int i = tamanhoTexto; i < newSize; i++) {
            texto[i] = ' ';
        }

    }
    struct AES_ctx ctx;
    AES_LoadKey(&ctx, key);
    //Loop de execuçâo do AES
    for(int i = 0; i < size; i += 16){
        AES_Start(&ctx, texto + i, 1);
        phex(texto + i);
    }
    CriarArquivo(texto, size);

    //Avisa ao usuario o termino do processo de criptografia
    GtkDialog *dialog = GTK_DIALOG(gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "CRIPTOGRAFADO!\nArquivo \"out.txt\" gerado"));
    gtk_dialog_run(dialog);
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

static void Decrypt_Main(char *path, char *key)
{
    unsigned char *texto;
    //toCharArray(LerString(), texto);
    int size;
    texto = LerArquivo(path, &size);

    struct AES_ctx ctx;
    AES_LoadKey(&ctx, (unsigned char*)key);
    //Loop de execuçâo do AES
    for(int i = 0; i < size; i += 16)
    {
        AES_Start(&ctx, texto + i, 0);
        printf("%.16s", texto + i);
    }
    CriarArquivo(texto, size);

    //Avisa ao usuario o termino do processo de descriptografia
    GtkDialog *dialog = GTK_DIALOG(gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "DESCRIPTOGRAFADO!\nArquivo \"out.txt\" gerado"));
    gtk_dialog_run(dialog);
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

//Função responsavel por ler um arquivo
static unsigned char *LerArquivo(char *path, int *size){
    unsigned char *buffer;

    FILE *arquivo;
    arquivo = fopen(path, "r");

    fseek(arquivo, 0L, SEEK_END);
    *size = ftell(arquivo);
    rewind(arquivo);
    buffer = malloc(*size + 1);
    if(!buffer)fclose(arquivo), fputs("ERRO DE ALOCAÇÃO DE MEMÓRIA!", stderr),exit(1);
    if(1!=fread(buffer, *size, 1, arquivo))
        printf("ERRO!");

    fclose(arquivo);
    return buffer;
}

//Função responsavel por criar um arquivo
static void *CriarArquivo(unsigned char *output, int size){
    FILE *arquivo;
    arquivo = fopen("out.aes", "w+");
    for(int i = 0; i < size; i+=16){
        fwrite(output + i, 1, 16, arquivo);
    }
    fclose(arquivo);
}
