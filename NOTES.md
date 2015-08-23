#Note ACSE

Di seguito alcune note che ho preso durante lo studio di ACSE.

### IMMEDIATE/REGISTER

Una espressione può essere immediate oppure register. In particolare è immediate quando il valore è immediato, altrimenti è register quando punta, con un identificatore, ad un registro.

In alcune (es: try catch) usa le strutture e crea il metodo per crearla, in altre, come ad esempio lo switch, le strutture ci sono ma manca il metodo
provato: sembra inutile il metodo per crearlo
nei token, alcuni (ad esempio EITHER) hanno davanti il tipo, es <either_stmt> oppure <label>. Altri no. Cosa cambia?
leggi in file Acse.y

    int zero = gen_load_immediate(0,IMMEDIATE);     // creo un int IMMEDIATE
    int one = getNewRegister(program);              // creo un int REGISTER
    gen_addi_instruction(program, imm_register, REG_0, 1);  

Posso poi creare un espressione passando un valore e il tipo:

    t_axe_expression expr= create_expression(zero,IMMEDIATE|REGISTER);

Si noti come l'istruzione `int imm_register = gen_load_immediate(program, 1)` equivalga a:

    int imm_register = getNewRegister(program);
    gen_addi_instruction(program, imm_register, REG_0, 1);   // 1 è int, REG_0 ha valore 0
    return imm_register;


### Gestione degli errori:
    printMessage("Positive immmediate expected”); 

Stampa la stringa come errore durante la compilazione

    notifyError(AXE_SYNTAX_ERROR); 

Notifica un errore di sintassi

    exit(-1) 
    abort(); 
    yyerror(); 

Si usano quando ci sono errori nella semantica, tipo si aspetta un valore positivo e invece ve nè uno negativo. Attenzione, al contrario dei precedenti queste istruzioni _bloccano la compilazione_

    gen_halt_instruction(program);

ACSE lo compila tranquillamente: ferma solo il programma .src quando si esegue (crea un halt in assembly)

Note:
t_axe_expression lt = handle_binary_comparison(program,$1,$3,_LT_); …come faccio ad usare il valore di un espressione in un if? if(lt.value) non funziona! lt  è una t_axe_epression. lt.value è il valore scalare ?


#### Istruzioni

La differenza tra gen_bmi/gen_blt e gen_bpl/gen_bgt non esiste se si usa come parametro di blt/bgt 0, infatti:

    gen_bmi_instruction(program,label,0); // branch on negative
    gen_bpl_instruction(program,label,0); // è branch on positive or 0


Le istruzioni in `axe_gencode.h` danno risultato anche nell’output register e sono verificabili tramite gen_beq ecc per i branch, le altre, contenute in `axe_expression.h` si limitano a fare le operazioni di confronti e operazioni numeriche. 

#### Liste

    t_forall_statement *stmt = (t_forall_statement*) getElementAt(forall_loop_nest,0)->data; 
equivale a

    t_forall_statement *stmt = (t_forall_statement*) LDATA(forall_loop_nest); 

questo perchè LDATA va a prendere il dato dalla lista (il primo, visto che una lista è un insieme di nodi e recupera il `data` del primo nodo)
