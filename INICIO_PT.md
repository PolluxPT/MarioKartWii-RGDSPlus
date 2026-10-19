# Começar — preparar Mario Kart Wii no Windows

## O resultado

Descarrega a release completa, extrai-a, coloca a tua ROM em **`ROM`** e abre **`PREPARAR_ROM.bat`**. O preparador converte os dados e cria **`PARA_CONSOLA/APPS`**, pronta para copiar para o cartão TF2. Não precisas de programar.

Na primeira utilização, instala as ferramentas abaixo. Elas não estão incluídas como programas executáveis nesta release; os links são das respetivas distribuições oficiais. O builder e o programa que coordena a preparação **estão incluídos**.

## 1. Preparar o PC uma única vez

Usa Windows 10/11 de 64 bits, ligação à Internet e pelo menos **20 GiB livres**. O processamento pode demorar uma hora ou mais. Extrai o ZIP inteiro para uma pasta simples, por exemplo `C:\MKW_RGDS`; não executes dentro do ZIP nem no cartão da consola.

Instala:

| Ferramenta | Como instalar |
|---|---|
| **Python 3.13.x de 64 bits** | [Python para Windows](https://www.python.org/downloads/windows/). Seleciona a série 3.13 e marca **Add python.exe to PATH**. O builder suporta 3.11–3.14; não uses 3.15. |
| **Git para Windows** | [Git](https://git-scm.com/downloads/win), com as opções normais do instalador. |
| **Node.js LTS** | [Node.js](https://nodejs.org/), incluindo npm. |
| **devkitPro** | [Instalador oficial](https://github.com/devkitPro/installer/releases/latest). Executa `devkitProUpdater-…exe`, seleciona **3DS Development** e mantém `C:\devkitPro`. O builder precisa da ferramenta `tex3ds`; não vais desenvolver um programa de 3DS. |

Depois de instalar, fecha eventuais janelas antigas de preparação. Abre **`VERIFICAR_FERRAMENTAS.bat`** e escolhe **1 — Português**. Corrige os itens assinalados como `EM FALTA` antes de continuar.

### Se a ROM estiver em RVZ

Obtém o [Dolphin para Windows](https://dolphin-emu.org/download/) e extrai a **distribuição completa** para `PC/Dolphin`. Pode ficar numa subpasta. Deve existir um `DolphinTool.exe` junto das suas DLL; não copies apenas esse executável isolado.

O preparador chama o DolphinTool para transformar RVZ em ISO e depois chama o builder. Não tens de abrir o emulador nem introduzir comandos. Para ISO e WBFS, esta ferramenta é dispensada.

## 2. Colocar a ROM

Coloca uma cópia tua de **Mario Kart Wii USA, identificador RMCE01**, em `ROM`. Aceita **ISO, WBFS ou RVZ**. O nome do ficheiro pode conter espaços e não precisa de ser alterado; o programa verifica o conteúdo.

Não uses a versão europeia/PAL (`RMCP01`), uma imagem modificada, NKit ou WBFS dividido em partes. A região correta não basta para garantir a revisão: o builder original verifica também ficheiros internos do disco e recusa os que não correspondem à revisão prevista.

## 3. Converter e preparar

1. Abre **`PREPARAR_ROM.bat`**.
2. Escolhe **1 — Português**. Se houver várias ROMs na pasta, escolhe o número da pretendida.
3. Aguarda as quatro etapas: formato do disco, dependências, conversão dos dados e montagem de APPS.
4. Só considera concluído quando aparecer **PRONTO** e o caminho `PARA_CONSOLA/APPS`.

As dependências Python e os projetos públicos fixados pelo builder são obtidos automaticamente pela Internet. Os ficheiros da tua ROM são processados localmente; o preparador não os envia. As mensagens técnicas do builder original podem aparecer em inglês.

O RVZ/ISO/WBFS original não é alterado. Um ISO temporário derivado de RVZ fica em `PC/work`. A cache e os ficheiros intermédios ficam em `PC/builder`; conserva-os se precisares de retomar. Ao repetir, o builder pode reutilizar etapas já concluídas.

## 4. Copiar para a RGDS+

1. Fecha o jogo e coloca o cartão TF2 no PC.
2. Copia o **conteúdo** de `PARA_CONSOLA/APPS` para a pasta `APPS` do cartão, aceitando juntar as pastas e substituir os ficheiros necessários.
3. Confirma estes caminhos no cartão:
   - `APPS/MarioKartWii.sh`
   - `APPS/MarioKartWii/3ds/mkw-3ds/content.txt`
   - `APPS/MarioKartWii/3ds/mkw-3ds/Race/Common.szs`
4. Volta a colocar o cartão e abre **APPS → MarioKartWii** na RGDS+.

Não copies a ROM original para a consola. Evita pastas duplicadas como `APPS/APPS`, `3ds/3ds` ou `mkw-3ds/mkw-3ds`.

## Ícone

Se tiveres um ícone, coloca-o em `APPS/Imgs/MarioKartWii.png` **antes de preparar**. Ele será copiado para a pasta pronta para a consola. Também podes adicioná-lo depois em `PARA_CONSOLA/APPS/Imgs/MarioKartWii.png`. A aplicação funciona sem imagem.

## Problemas

- Ferramenta em falta: executa `VERIFICAR_FERRAMENTAS.bat` e consulta a tabela acima.
- Erro ou interrupção na preparação: guarda **`PC/Preparacao.txt`** e volta a executar. Não apagues a cache inteira como primeiro passo.
- Falha só na consola: guarda `APPS/MarioKartWii/Arranque.txt` e `Jogo.txt`.
- Só aparecem cores: a aplicação não encontrou os dados convertidos nos caminhos indicados.
- Não há som: sobe o volume com as teclas físicas; nível zero é silêncio.

Não publiques a ROM, `PC/work`, `PC/builder` ou `PARA_CONSOLA` no GitHub. A release distribuível é criada a partir dos ficheiros limpos do projeto e exclui os dados de utilizador.

Esta versão tem aplicação e jogabilidade testadas na RGDS+ Stock Linux. O novo coordenador PT/EN foi validado com testes automatizados e simulações; uma conversão integral nova não pôde ser repetida sem a ROM. O motor e o builder incluídos são os da base original usada neste projeto.
