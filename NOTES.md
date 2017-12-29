# Note ACSE

Di seguito alcune note che ho preso durante lo studio di ACSE.

### IMMEDIATE/REGISTER

Una espressione può essere immediate oppure register. In particolare è immediate quando il valore è immediato, altrimenti è register quando punta, con un identificatore, ad un registro.
	 
	 // carico un IMMEDIATE, ritorna l'identificatore del registro
    int zero = gen_load_immediate(0,IMMEDIATE);    
    
    // prende un nuovo registro dove salvarci int, ritorna identificatore
    int one = getNewRegister(program);              
    gen_addi_instruction(program, one, REG_0, 1);   // posso usarlo per inserirci valori

Posso poi creare un espressione passando un valore e il tipo:

    t_axe_expression expr= create_expression(value,IMMEDIATE|REGISTER);

Una volta che ho creato l'espressione, posso recuperarne il valore facendo `exp.value`. Chiaramente, come detto prima, il valore può essere sia immediato che un identificatore di un registro.

Esiste dunque  `t_axe_expression exp = create_expression(0,IMMEDIATE) ` che crea una exp con quel valore immediato, mentre se ha  `REGISTER ` e un valore tipo  `int zero ` allora punta solo al registro (le modifiche sono associate, è un puntatore).

Si noti come l'istruzione `int imm_register = gen_load_immediate(program, 1)` equivalga a:

    int imm_register = getNewRegister(program);
    gen_addi_instruction(program, imm_register, REG_0, 1);   // 1 è int, REG_0 ha valore 0
    return imm_register;

Spesso è comodo un pattern per caricare le `exp`:
    
    int disp;
    // carico valore displacement, che è una exp
    if(displacement.expression_type == IMMEDIATE) {
        disp = gen_load_immediate(program,displacement.value);
    } else {
        disp = getNewRegister(program);
        disp = gen_andb_instruction(program,disp,displacement.value,displacement.value,CG_DIRECT_ALL);
    }
    // genero exp per il displacement (cosi è utilizzabile in istruzioni che lo richiedono
    // attenzione, visto che ho messo il tipo REGISTER, ora modificando disp_exp, anche il valore
    // di disp cambia! 
    
    t_axe_expression disp_exp = create_expression(disp,REGISTER);

Con gli identifier, se vi è bisogno di caricarli:

    int iv_reg = get_symbol_location(program,$2,0);
    // posso poi usarlo, ad esempio:
    gen_andb_instruction(program,iv_reg,iv_reg,iv_reg,CG_DIRECT_ALL)
    
Quindi: 

- `NUMBER` sono `int`
- `IDENTIFIER` sono `ID`, si prendono con `getVariable(program, char *ID)` solitamente sono nomi di variabili (anche array)
- `exp` sono espressioni e vanno valutate se `IMMEDIATE` o meno


    
    
### Operazioni

`gen_load_immediate, gen_andb_instruction [...]` scrivono sul PSW
`gen_handle_binary_comparison ` scrive sul PSW

`gen_handle_bin_numeric:op [...]` _NON_ scrive sul PSW

### Istruzioni

- La differenza tra gen_bmi/gen_blt e gen_bpl/gen_bgt non esiste se si usa come parametro di blt/bgt 0, infatti:
    `gen_bmi_instruction(program,label,0); // branch on negative`
    `gen_bpl_instruction(program,label,0); // è branch on positive or 0`


- Le istruzioni in `axe_gencode.h` danno risultato anche nell’output register e sono verificabili tramite gen_beq ecc per i branch, le altre, contenute in `axe_expression.h` si limitano a fare le operazioni di confronti e operazioni numeriche. 

- In alcuni esempi (es: try catch) usa le strutture e scrive il metodo per crearle, in altre, come ad esempio lo switch, le strutture ci sono ma manca il metodo: spesso e volentieri, non è necessario scrivere il metodo che inizializza a NULL le labels della struct.

- Nei tokens, alcuni sono preceduti dalla struttura che devono chiamare. 
  Ad esempio, il WHILE ha in fronte <while_stmt> che va ad istanziare la relativa struttura. Ci si può referenziare a questa usando $1 nella grammatica, andando a recuperare le variabili della struttura stessa ($1 perchè, in questo caso, è il primo parametro). 
  Altre, ad esempio il DO, non hanno bisogno di una struttura complessa ma di una sola label, perciò si utilizza semplicemente <label> e ci si riferenzia ad essa come $1. Si può infatti creare con $1 = newLabel(program).
  Altre ancora, non hanno nessuna "entità" in fronte: questo perchè non necessitanodi label per fare dei jump o simili (esempio: RETURN, WRITE...)

### Espressioni

     t_axe_expression e_zero = create_expression(0,IMMEDIATE);
     t_axe_expression e_lenght = create_expression(id->arraySize,IMMEDIATE);
     
### Array

     t_axe_variable *id = getVariable(program,$1);
     id->isArray // è array o meno?
     id->arraySize // lunghezza array
	
	  array->ID // si passa alle funzioni qui sotto:
     
     int element = loadArrayElement(program,array->ID,index_exp);
     storeArrayElement(program,array->ID,index_exp,data_exp);
     
Si può usare `id->arraySize` come intero e svolgerci sopra operazioni: 
 
    int last_el = id->arraySize-1

Quando passo degli array a delle funzioni, è meglio recuperarli con:
     
     t_axe_variable *dest_array = getVariable(program,$1);
     t_axe_variable *src_array = getVariable(program,$3);

### Liste

    t_forall_statement *stmt = (t_forall_statement*) getElementAt(forall_loop_nest,0)->data; 
equivale a

    t_forall_statement *stmt = (t_forall_statement*) LDATA(forall_loop_nest); 

questo perchè LDATA va a prendere il dato dalla lista (il primo, visto che una lista è un insieme di nodi e recupera il `data` del primo nodo)

Per aggiungere/rimuovere un elemento, ad esempio una label

    lista = addFirst(lista, elemento) 
    // elemento deve essere puntatore (precedere con & se non ha *)
    
    lista = removeFirst(lista)
    
Istruzioni:

    list = list->next // va al prossimo, anche LNEXT
    list = list->data // ottiene il dato in testa LDATA
    
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

### Tipi

Ricordati, che se serve una struttura, puoi fare 

    %token <switch_stmt> SWITCH
    [...]
    // aggiungere in %union nei SEMANTIC RECORDS
    t_switch_statement switch_stmt; (ricordare il * se va in una lista)
    [...]
    switch_statement: SWITCH [...] {
    			$1.label_end = newLabel(program);
    [...]
    
Se serve solo una label, si può fare:

    %token <label> ON
    [...]
    onflag_statement: ON [...] {
    			$1 = newLabel(program);
    [...]
    
Se in uno statement ti usa ad esempio uno statement che poi è una lista o altra struttura (expr_list), serve aggiungere dopo i vari token.

    %type <list> expr_list

Si prenda come esempio
		
		// tipi
		%type <list> perm_list
		%type <permutation_element> perm_elem
		
		// statements
		perm_list : perm_list COMMA perm_elem 
		  {
				t_axe_permutation_element* last = (t_axe_perm_el *) LDATA(getLastElement($1));
				last->destination = $3->source;
				$$ = addLast($1, $3);
		  }
		  | perm_elem {
				$$ = addLast(NULL, $1);
		  }
		;
		
		perm_elem : NUMBER { // in questo caso solo NUMBER! OCCHIO!
			// t_axe_permutation non viene istanziata sopra, quindi crearla
			t_axe_perm_el* result = (t_axe_perm_el*) _AXE_ALLOC_FUNCTION(sizeof (t_axe_perm_el));
			result->source = $1;
			result->destination = 0;
			$$ = result;
			}	
		;
		
		// struttura in axe_struct.h
		typedef struct t_axe_permutation_element
		{
		   int source;
		   int destination;
		} t_axe_permutation_element;

    
### Costanti

Per definire costanti, ad esempio `LEFT` e `RIGHT` in constants.h

    #define LEFT 100
    #define RIGHT 101
