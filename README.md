Το πρόγραμμα αποτελείται από τρία βασικά μέρη: τον server, τους clients, και τον κοινό ορισμό του συστήματος (shop.h).

1. shop.h
Περιέχει τις κοινές δηλώσεις και τις δομές που χρησιμοποιούνται από τον server και τους clients.
Δημιουργείται ένας πίνακας προϊόντων, όπου κάθε προϊόν έχει περιγραφή, τιμή, και διαθέσιμη ποσότητα.
2. Server
Ο server υλοποιεί το ηλεκτρονικό κατάστημα:

Αρχικοποίηση Καταλόγου: Δημιουργεί έναν πίνακα 20 προϊόντων με προκαθορισμένα χαρακτηριστικά.
Διαχείριση Παραγγελιών:
Δέχεται αιτήματα από πελάτες μέσω sockets.
Ελέγχει αν το προϊόν είναι διαθέσιμο και εξυπηρετεί ή απορρίπτει το αίτημα.
Χρησιμοποιεί semaphores για να διασφαλίσει ότι τα δεδομένα του καταλόγου είναι συνεπή.
Αναφορές:
Στο τέλος, ο server τυπώνει μια αναφορά που περιλαμβάνει στατιστικά στοιχεία για τις παραγγελίες και τον τζίρο.
3. Client
Κάθε client υποβάλλει παραγγελίες:

Σύνδεση με τον Server: Συνδέεται μέσω socket.
Υποβολή Παραγγελιών:
Επιλέγει τυχαία προϊόντα και ποσότητες.
Στέλνει την παραγγελία στον server και λαμβάνει απάντηση.
Πολλαπλοί Πελάτες:
Με χρήση fork(), δημιουργούνται 5 clients που λειτουργούν ταυτόχρονα.
