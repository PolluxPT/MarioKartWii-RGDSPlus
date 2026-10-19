# Configuração

O ficheiro `APPS/MarioKartWii/config.txt` contém as opções da aplicação.

| Opção | Valor final | Efeito |
|---|---|---|
| `MKW_OUTPUT` | `0` | Ecrã superior no perfil Stock testado |
| `MKW_FB_WIDTH` / `MKW_FB_HEIGHT` | `1024` / `768` | Resolução da janela |
| `MKW_LAYOUT` | `fit` | Mantém as proporções; `full` estica para preencher |
| `MKW_SCALE` | `1` | Resolução interna original |
| `MKW_AUDIO_GAIN` | `0.35` | Teto do ganho; `0.20` reduz mais o volume |
| `MKW_LOWER_OFF` | `1` | Apaga a iluminação inferior; `0` mantém ligada |
| `MKW_TEST_SECONDS` | `0` | Sem limite automático de duração |

O volume segue o nível Stock de 0 a 10. O ganho é `MKW_AUDIO_GAIN × (nível/10)²`. Zero corresponde a silêncio.

O perfil testado identifica a iluminação superior como `backlight1` e a inferior como `backlight`. A luminosidade inferior anterior é restaurada quando o processo de arranque termina normalmente, por erro do jogo ou por um sinal tratado pelo script.

O ficheiro `controls.txt` permite alterar ações; `controls.default.txt` conserva a configuração final de referência. L3 e R3 continuam configurados como alternativas de câmara caso o equipamento exponha esses cliques; as funções principais usam L2 e R2.
