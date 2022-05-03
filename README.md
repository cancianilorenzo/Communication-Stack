# TRAP_implementation
Implementation of TRAP protocol for nodes communication for my thesis project </br>
TODO: </br>

-fix node selection (node[0], [1], [2]) </br>


FIX BURST Reception</br>


15khz --> circa 5376 sul timer </br>
25khz --> circa 3230 sul timer </br>
35Khz --> circa 2313 sul timer </br>

(Attuale velocità SMCLK 92165hz???????????) Need to test clock speed!!!! Non è possibile, non potrebbe ricevere interrupt a a frequenze maggiori di 9khz!



Il problema è il blocco nell'attesa durante la ricezione --> non resetta contatore



16000000/[(Tick totali/6)] ------------------------ Formula giusta, il clock va a 16000000Hz (16MHz) come settato
