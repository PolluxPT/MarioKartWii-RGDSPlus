# Publicar no GitHub

Nome sugerido do repositório: **MarioKartWii-RGDSPlus**.

## 1. Acrescentar a imagem

Antes de publicares, coloca o teu PNG em:

`APPS/Imgs/MarioKartWii.png`

Não precisas de alterar o programa nem o nome do script. Não substituas a imagem por um ficheiro de texto com extensão `.png`: usa o PNG verdadeiro.

## 2. Criar o ZIP final

No Windows, abre `Criar_ZIP.bat`. É necessário Python 3.11 ou posterior. O ZIP é criado em:

`dist/MarioKartWii_RGDSPlus_v1.1.0.zip`

O script inclui automaticamente a imagem se existir. Sem imagem, cria o mesmo instalador e informa que falta apenas o ícone opcional. Não adiciona ROMs nem os teus dados convertidos: se os encontrar em `APPS`, interrompe o empacotamento.

## 3. Criar o repositório

1. Entra na tua conta em [github.com](https://github.com).
2. Usa **New repository**, com o nome sugerido acima e a visibilidade que pretendes.
3. Usa **Add file → Upload files** para carregar os ficheiros e pastas deste projeto.
4. Carrega o **conteúdo** da pasta `MarioKartWii-RGDSPlus`, não o ZIP exterior.
5. Deixa a pasta `dist` fora dos ficheiros do repositório: o seu ZIP será anexado à release.

Publica a partir de uma cópia limpa deste pacote. Não carregues a ROM, `PARA_CONSOLA`, `PC/work`, `PC/builder` extraído ou dados convertidos. A pasta `APPS/MarioKartWii/3ds` no projeto contém apenas instruções de instalação.

## 4. Criar a release

1. Abre **Releases → Draft a new release**.
2. Cria a etiqueta `v1.1.0`.
3. Título: **Mario Kart Wii for RGDS+ — v1.1.0**.
4. Usa o texto de `RELEASE_NOTES.md` como descrição.
5. Anexa o ZIP criado em `dist` e publica a release.

O ZIP exterior de preparação contém todo o repositório e uma cópia inicial do instalador em `dist`. Se acrescentares o ícone, volta a executar `Criar_ZIP.bat` antes de anexar o instalador à release.

A release v1.1.0 inclui também `ROM`, o preparador PT/EN, `PC/builder.zip` e os guias de preparação. Quem fizer download não precisa de procurar o ZIP original desta conversa. As ferramentas de sistema são instaladas conforme os guias, uma única vez.
