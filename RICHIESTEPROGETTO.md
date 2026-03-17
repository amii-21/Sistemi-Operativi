5 Descrizione del progetto: versione minima (voto max 24 su 30)
Si intende simulare il funzionamento di una mensa studentesca/aziendale. A tal fine sono presenti i seguenti
processi e risorse: cassa di pagamento, stazioni di distribuzione cibo, tavoli, bancone del caff`e, cassiere, operatori e
responsabile mensa, studente/utente.
• Processo responsabile mensa che gestisce la simulazione, e mantiene le statistiche su richieste e servizi erogati
dalla mensa. Genera tutti gli altri processi del simulatore.
• Processo operatore stazione primi: eroga i primi piatti. In particolare, dovranno essere implementate le
funzionalit`a per garantire la scelta tra almeno 2 primi piatti ogni giorno. Il tempo medio per erogare il servizio
`e AVG SRVC PRIMI secondi, e dovr`a essere usato per generare un tempo casuale di erogazione nell’intorno ±50%
del valore indicato.
• Processo operatore stazione secondi: eroga i secondi piatti e i contorni. In particolare, dovranno essere
implementate le funzionalit`a per garantire almeno la scelta tra 2 secondi piatti con relativi contorni ogni
giorno. Il tempo medio per erogare il servizio `e AVG SRVC MAIN COURSE secondi, e dovr`a essere usato per
generare un tempo casuale di erogazione nell’intorno ±50% del valore indicato.
• Processo operatore stazione coffee: eroga i caff`e e il dolce. In particolare, dovranno essere implementate
le funzionalit`a per garantire almeno la scelta tra 4 tipi di caff`e (normale, macchiato, decaffeinato, ginseng). Il
tempo medio per erogare il servizio `e AVG SRVC COFFEE secondi, e dovr`a essere usato per generare un tempo
casuale di erogazione nell’intorno ±80% del valore indicato.
3
• Processo operatore cassa (cassiere): gestisce i pagamenti degli utenti. Il tempo medio per erogare il ser-
vizio `e AVG SRVC CASSA secondi, e dovr`a essere usato per generare un tempo casuale di erogazione nell’intorno
±20% del valore indicato.
• Esistono NOF WORKERS processi di tipo operatore: hanno un orario di lavoro, ed effettuano pause casuali.
• Processi di tipo utente. In base alla situazione delle code alle varie stazioni di erogazione cibo, l’utente
decide in quale coda inserirsi; ottenuto il piatto, decide se procedere alla cassa o passare a un’altra stazione di
distribuzione cibo, e poi recarsi alla cassa. Una volta pagati alla cassa i piatti ottenuti, mangia per un tempo
proporzionale al numero di piatti acquistati, e poi torna a casa/ufficio/aula.
– Prima di iniziare a consumare i pasti, gli utenti devono passare a pagare presso la stazione cassa.
• Esistono NOF USERS processi di tipo utente. Il processo utente decide se recarsi in mensa e sceglie cosa
mangiare (si veda la descrizione del processo nella sezione 5.5).
• Esistono risorse di tipo stazione di distribuzione cibo e stazioni di refezione (o, meno pomposa-
mente, tavoli); ogni stazione `e destinata a fornire un solo tipo di cibo (primo/secondo-contorno/dolce-caff`e),
oppure a consentire agli utenti di consumare il cibo acquistato. Esiste inoltre un altro tipo di stazione, la
stazione cassa.
• Ogni stazione di distribuzione `e dotata di un certo numero di postazioni: NOF WK SEATS PRIMI, NOF WK SEATS SE-
CONDI, NOF WK SEATS COFFEE, NOF WK SEATS CASSA.
5.1 Processo responsabile mensa
Il processo responsabile mensa `e responsabile dell’avvio della simulazione, della creazione delle risorse di tipo
stazione distribuzione e stazione refezione, dei processi operatore e utente, delle statistiche e della termi-
nazione. Si noti bene che il processo responsabile mensa non si occupa dell’aggiornamento delle statistiche, ma
solo della loro lettura, secondo quanto indicato. All’avvio, il processo responsabile mensa:
• crea NOF WORKERS processi di tipo operatore e operatore cassa, assegnandoli alle stazioni di distribuzione
o alle casse, garantendo che nessuna stazione rimanga senza operatore.
• crea NOF USERS processi di tipo utente.
• crea le 4 stazioni distribuzione per primi, secondi, caff`e e la cassa.
• crea NOF WK SEATS PRIMI, NOF WK SEATS SECONDI, NOF WK SEATS COFFEE, NOF WK SEATS CASSA risorse di tipo
postazione, per la corrispondente stazione di distribuzione.
• crea NOF TABLE SEATS risorse di tipo stazione refezione.
Successivamente il responsabile mensa avvia la simulazione, che avr`a come durata SIM DURATION giorni, dove
ciascun minuto `e simulato dal trascorrere di N NANO SECS nanosecondi.
La simulazione deve cominciare solamente quando tutti i processi cassiere, operatore e utente sono stati creati
e hanno terminato la fase di inizializzazione.
Alla fine di ogni giornata, il processo responsabile mensa dovr`a stampare le statistiche totali e quelle della
giornata, che comprendono:
• il numero di utenti serviti totali nella simulazione;
• il numero di utenti non serviti totali nella simulazione;
• il numero di utenti serviti in media al giorno;
• il numero di utenti non serviti in media al giorno;
• il numero di piatti distribuiti totali e per tipo (primo, secondo-contorno, dolce-caff`e) nella simulazione;
4
• il numero di piatti avanzati totali e per tipo (primo, secondo-contorno, dolce-caff`e) nella simulazione;
• il numero di piatti distribuiti in media (totali e per tipo) al giorno;
• il numero di piatti avanzati in media (totali e per tipo) al giorno;
• il tempo medio (complessivo e su ciascuna stazione) di attesa degli utenti nella simulazione;
• il tempo medio di attesa (complessivo e su ciascuna stazione) degli utenti nella giornata;
• il numero di operatori attivi durante la giornata. Un operatore si considera attivo se `e riuscito ad accedere
ad una postazione;
• il numero di operatori attivi durante la simulazione;
• il numero medio di pause effettuate nella giornata e il totale di pause effettuate durante la simulazione;
• il ricavo giornaliero;
• il ricavo totale su tutta la simulazione e medio per ciascuna giornata (a fine simulazione).
5.2 Risorse di tipo stazione
All’inizio della giornata, gli operatori sono associati a una stazione (di tipo distribuzione o cassa) dal responsabi-
le mensa che assegna sempre almeno un operatore a ciascuna stazione, e poi utilizza come criterio preferenziale
quello di assegnare pi`u operatori a mansioni il cui tempo medio di svolgimento (AVG SRVC ∗) `e pi`u alto. La politica
di associazione operatore-servizio `e definita dal progettista, e deve essere applicata all’inizio di ogni giornata.
Le stazioni di refezione dei primi e dei secondi vengono inizializzate –all’inizio di ogni giorno– con un numero
pari a AVG REFILL PRIMI e AVG REFILL SECONDI di porzioni per ciascun tipo di piatto; e poi alimentate, ogni 10
minuti, fino ad un massimo numero di porzioni MAX PORZIONI PRIMI e MAX PORZIONI SECONDI, uguale per tutti i
tipi di piatti.
La stazione dolce e caff`e ha invece una quantit`a illimitata di porzioni.
5.3 Processo operatore
All’inizio di ogni giornata lavorativa, l’operatore:
• compete con gli altri operatori per il posto presso la stazione a cui il responsabile mensa l’ha assegnato;
• se ne trova uno, comincia il proprio turno che terminer`a alla fine della giornata lavorativa;
• con un massimo di NOF PAUSE volte durante ciascuna giornata, l’operatore pu`o decidere (secondo un criterio
scelto dal programmatore) di interrompere il servizio per poi riprendere il proprio compito dopo la pausa.
Non tutti gli operatori possono andare in pausa contemporaneamente: non `e consentito agli operatori lasciare
la stazione non presidiata (almeno un operatore deve sempre essere attivo). In caso un operatore decida di
mettersi in pausa:
– termina di servire il cliente che stava servendo;
– lascia libera la stazione occupata;
– aggiorna le statistiche.
Il processo operatore che al suo arrivo non trova una postazione libera resta in attesa che una postazione si liberi
(per una pausa di un altro operatore).
5.4 Processo operatore cassa
Il processo cassiere `e responsabile dei pagamenti: gli utenti che si presentano alla cassa indicano quali piatti hanno
scelto e se vogliono il caff`e, e il cassiere aggiorna il ricavo totale. Il costo di ciascun primo `e PRICE PRIMI, il costo
di ciascun secondo `e PRICE SECONDI, e il prezzo del caff`e `e PRICE COFFEE.
5
5.5 Processo utente
I processi di tipo utente si recano presso la mensa. Pi`u in dettaglio, ogni giorno ogni processo utente:
• Stabilisce il menu che vuole acquistare (secondo un criterio stabilito dal progettista); ad esempio legge il
file di configurazione menu.txt in cui sono elencate diverse alternative, e contiene almeno due primi e due
secondi, oltre a dolce e caff`e. Il file di configurazione deve rispettare una sintassi definita dal progettista, e la
composizione del menu potr`a essere modificata in sede di esame;
• In base alla composizione del menu scelto –che deve includere almeno un primo, un secondo ed eventualmente
il dolce-caff`e– si reca alle stazioni che forniscono i prodotti desiderati;
• Attende il proprio turno e l’erogazione del piatto presso ciascuna stazione di distribuzione;
• Al momento in cui l’utente sta per essere servito pu`o capitare che il piatto desiderato sia terminato: in questo
caso l’utente sceglier`a un altro piatto dello stesso tipo (primi o secondi). Se tutti i piatti di quel tipo sono
terminati, manger`a solo secondo o solo primo. Se tutti i piatti di entrambi i tipi sono terminati, desiste ed
esce;
• Dopo avere ottenuto i piatti scelti, passa in cassa e paga;
• Si siede in una stazione di refezione dove consuma il pasto acquistato. All’interno della mensa esiste un
numero di posti complessivo NOF TABLE SEATS: per potersi sedere a un tavolo, l’utente deve verificare che ci
sia almeno un posto libero; altrimenti attende che si liberi un posto.
Se al termine della giornata l’utente si trova ancora in coda, abbandona la mensa rinunciando al pasto. Il numero
di utenti che rinunciano `e uno dei parametri da monitorare.
5.6 Terminazione
La simulazione termina in una delle seguenti circorstanze:
timeout raggiungimento della durata impostata a SIM DURATION giorni;
overload numero di utenti in attesa al termine della giornata maggiore del valore OVERLOAD THRESHOLD.
Il gruppo di studenti deve produrre e consegnare, nell’archivio contenente il progetto, diverse configurazioni (file
config timeout.conf e config overload.conf) in grado di generare la terminazione nei casi sopra descritti.
Al termine della simulazione, l’output del programma deve riportare anche la causa di terminazione e le
statistiche finali.
