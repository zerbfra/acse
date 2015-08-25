#Note ACSE

Di seguito alcune note che ho preso durante lo studio di ACSE.

### IMMEDIATE/REGISTER

Una espressione può essere immediate oppure register. In particolare è immediate quando il valore è immediato, altrimenti è register quando punta, con un identificatore, ad un registro.

    int zero = gen_load_immediate(0,IMMEDIATE);     // carico un IMMEDIATE, ritorna l'identificatore del registro
    int one = getNewRegister(program);              // prende un nuovo registro dove salvarci int, ritorna identificatore
    gen_addi_instruction(program, one, REG_0, 1);   // posso usarlo per inserirci valori

Posso poi creare un espressione passando un valore e il tipo:

    t_axe_expression expr= create_expression(value,IMMEDIATE|REGISTER);

Una volta che ho creato l'espressione, posso recuperarne il valore facendo `exp.value`. Chiaramente, come detto prima, il valore può essere sia immediato che un identificatore di un registro.

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

- La differenza tra gen_bmi/gen_blt e gen_bpl/gen_bgt non esiste se si usa come parametro di blt/bgt 0, infatti:
    `gen_bmi_instruction(program,label,0); // branch on negative`
    `gen_bpl_instruction(program,label,0); // è branch on positive or 0`


- Le istruzioni in `axe_gencode.h` danno risultato anche nell’output register e sono verificabili tramite gen_beq ecc per i branch, le altre, contenute in `axe_expression.h` si limitano a fare le operazioni di confronti e operazioni numeriche. 

- In alcuni esempi (es: try catch) usa le strutture e scrive il metodo per crearle, in altre, come ad esempio lo switch, le strutture ci sono ma manca il metodo: spesso e volentieri, non è necessario scrivere il metodo che inizializza a NULL le labels della struct.

- Nei tokens, alcuni sono preceduti dalla struttura che devono chiamare. 
  Ad esempio, il WHILE ha in fronte <while_stmt> che va ad istanziare la relativa struttura. Ci si può referenziare a questa usando $1 nella grammatica, andando a recuperare le variabili della struttura stessa ($1 perchè, in questo caso, è il primo parametro). 
  Altre, ad esempio il DO, non hanno bisogno di una struttura complessa ma di una sola label, perciò si utilizza semplicemente <label> e ci si riferenzia ad essa come $1. Si può infatti creare con $1 = newLabel(program).
  Altre ancora, non hanno nessuna "entità" in fronte: questo perchè non necessitanodi label per fare dei jump o simili (esempio: RETURN, WRITE...)

#### Espressioni

     t_axe_expression e_zero = create_expression(0,IMMEDIATE);
     t_axe_expression e_lenght = create_expression(id->arraySize,IMMEDIATE);
     
#### Array

     t_axe_variable *id = getVariable(program,$1);
     id->isArray // è array o meno?
     id->arraySize // lunghezza array
     
     int element = loadArrayElement(program,array->ID,index_exp);
     storeArrayElement(program,array->ID,index_exp,data_exp);

#### Liste

    t_forall_statement *stmt = (t_forall_statement*) getElementAt(forall_loop_nest,0)->data; 
equivale a

    t_forall_statement *stmt = (t_forall_statement*) LDATA(forall_loop_nest); 

questo perchè LDATA va a prendere il dato dalla lista (il primo, visto che una lista è un insieme di nodi e recupera il `data` del primo nodo)
