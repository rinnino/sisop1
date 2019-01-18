/* @author: Simone Sollena
 * Raccolta di sequenze di escape per stampe colorate su terminale.
 * Esempio di utilizzo:
 * printf("Hello %sWorld%s!",COLOR_RED , COLOR_NRM);
 * stampa "Hello World!" con la parola World in rosso, torna a stampare
 * con colore normale il punto esclamativo finale.
 */

#ifndef _COLORS_H_
#define _COLORS_H_

/** Sequenza di escape per formato testo normale */
#define COLOR_NRM  "\x1B[0m"
/** Sequenza di escape per formato testo grassetto */
#define COLOR_BLD  "\x1B[1m"
/** Sequenza di escape per formato testo blink/lampeggiante */
#define COLOR_BLNK "\x1B[5m"
/** Sequenza di escape per formato testo nascosto */
#define COLOR_HIDE "\x1B[8m"
/** Sequenza di escape per formato testo inverso */
#define COLOR_RVRS "\x1B[7m"
/** Sequenza di escape per formato testo nero */
#define COLOR_BLK  "\x1B[30m"
/** Sequenza di escape per formato testo rosso */
#define COLOR_RED  "\x1B[31m"
/** Sequenza di escape per formato testo verde */
#define COLOR_GRN  "\x1B[32m"
/** Sequenza di escape per formato testo giallo */
#define COLOR_YEL  "\x1B[33m"
/** Sequenza di escape per formato testo blu */
#define COLOR_BLU  "\x1B[34m"
/** Sequenza di escape per formato testo magenta */
#define COLOR_MAG  "\x1B[35m"
/** Sequenza di escape per formato testo ciano */
#define COLOR_CYN  "\x1B[36m"
/** Sequenza di escape per formato testo bianco */
#define COLOR_WHT  "\x1B[37m"

/** Sposta il cursore alla colonna indicata */
#define movecursoratcol(n) printf("\r\x1B[%dC",n)

#endif
