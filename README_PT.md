# Mario Kart Wii for RGDS+

Adaptação do remake **MKW-3DS** para a **ANBERNIC RGDS+ com Stock Linux**, preparada e testada com Filipe Baeta.

Este projeto adapta o remake de MiniBandit e o port H700 de Matrioshka Brain. É específico deste remake de Mario Kart Wii; não é um emulador de Wii nem um motor genérico para outros jogos.

## Instalação e preparação da ROM

Descarrega a **release completa v1.1.0**, extrai-a no PC e segue [INICIO_PT.md](INICIO_PT.md).

1. Na primeira utilização, instala Python 3.13, Git, Node.js LTS e devkitPro/tex3ds conforme o guia. Para RVZ, prepara também o DolphinTool.
2. Coloca a tua ROM americana **RMCE01** em `ROM` (ISO, WBFS ou RVZ).
3. Abre `PREPARAR_ROM.bat` e escolhe Português.
4. Quando aparecer **PRONTO**, copia o conteúdo de `PARA_CONSOLA/APPS` para `APPS` do cartão TF2.
5. Na consola, abre **APPS → MarioKartWii**.

O preparador PT/EN e o builder H700 original estão incluídos. A preparação de dependências públicas e os dados do jogo é coordenada pelo script. As ferramentas de sistema da primeira utilização têm instalação separada, documentada no guia; não é preciso programar.

Não inclui ROM nem dados extraídos do jogo. Uma ROM europeia/PAL ou uma revisão não aceite pelo builder não é compatível. Os dados da ROM são processados no teu PC. A conversão pode demorar uma hora ou mais.

Se já tens os dados convertidos e a aplicação funcional, podes sobrepor `APPS` na consola preservando `3ds/mkw-3ds`; não precisas de repetir a preparação.

## Funcionalidades

- Saída no ecrã superior a 1024×768, mantendo as proporções nativas.
- Mapeamento de comandos da RGDS+.
- Volume ajustável pelas teclas físicas, com curva suave nos níveis baixos.
- Iluminação inferior desligada durante o jogo e restaurada ao sair.
- Visão traseira, truques direcionais e configurações editáveis.
- Registos de arranque dentro da pasta da aplicação.

## Comandos

| Botão | Função |
|---|---|
| A / B | Acelerar e confirmar / travar e voltar |
| Analógico esquerdo ou D-pad | Direção |
| L1 / R1 | Usar item / salto e derrapagem |
| L2 mantido | Visão traseira |
| R2 ou X + direção no analógico | Truque na rampa |
| Y, neutro ou para cima | Cavalinho na mota |
| Y + analógico para baixo | Pedir fim do cavalinho |
| Start / Select | Pausa / indicadores |
| Start + Select | Sair |
| Volume + / − | Ajustar o som |

Ver [guia de configuração](docs/CONFIGURACAO.md). A elegibilidade e a animação das manobras dependem do remake. O cavalinho não foi confirmado no teste final porque as motas ainda não estavam desbloqueadas.

## Compatibilidade e estado

A aplicação final 1.0, incluída no pacote 1.1.0, reúne a versão final testada na **RGDS+ Stock Linux**. Imagem, comandos, som e jogabilidade foram confirmados pelo utilizador. Não foi testada noutros modelos ou sistemas. O desempenho e as limitações do remake podem variar com a pista e a situação.

Ao reportar problemas, indica a versão de firmware e anexa `APPS/MarioKartWii/Arranque.txt` e `Jogo.txt`, quando existirem. Não envies ROMs nem pastas de dados do jogo.

## Ícone

O caminho previsto é **`APPS/Imgs/MarioKartWii.png`**, com o mesmo nome-base de `MarioKartWii.sh`. O ícone é opcional e não vem incluído neste pacote. Podes adicionar o teu PNG nessa pasta e reconstruir o ZIP com `Criar_ZIP.bat`. Ver [publicação](docs/PUBLICAR_NO_GITHUB.md).

## Fontes e criação da release completa

As fontes da adaptação estão em `Sources`. O executável H700 foi mantido sem alterações e a ponte identifica a versão compatível antes de chamar as suas APIs. O arquivo H700 fornecido tem a etiqueta **v1.21**, enquanto o executável fornecido tem a etiqueta **v1.22**: não se afirma que aquele arquivo reproduz exatamente este executável.

- Compilar a ponte e auxiliares: [BUILD.txt](Sources/BUILD.txt).
- Criar o ZIP completo (aplicação + preparador + instruções): `python tools/package_release.py` ou `Criar_ZIP.bat`.
- Preparar a publicação: [guia em português](docs/PUBLICAR_NO_GITHUB.md).

## Créditos e licenças

- **MiniBandit** — [MKW-3DS](https://github.com/MiniBandit/MKW-3DS), incluindo os créditos ao trabalho original de dudewhat.
- **Matrioshka Brain** — port H700 e camada de compatibilidade, conforme os avisos do pacote original.
- **Filipe Baeta** — projeto de adaptação e testes na RGDS+.
- Dependências e avisos: [THIRD_PARTY.md](THIRD_PARTY.md) e `APPS/MarioKartWii/licenses`.

A licença [MIT](LICENSE) cobre as adições da adaptação RGDS+; não substitui as licenças dos componentes de terceiros nem atribui direitos sobre a imagem que possas acrescentar. Mario Kart Wii e as respetivas personagens pertencem à Nintendo. Este projeto não é afiliado à Nintendo ou à ANBERNIC.

