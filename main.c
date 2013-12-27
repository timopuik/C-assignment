/*
* Kurssi: C-ohjelmointi
* Harjoitustyon nimi: Pankkiautomaatti
* Tekija: Timo Puikko
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <gmp.h>
#include <ctype.h>

/* 
* struktin sisällä ovat char-taulukko tilinumero, johon luetaan käyttäjän syötteestä tilin numero jonka 
* avulla taas avataan tilitiedosto, integer-muuttuja tunnusluku, joka luetaan tilitiedostosta. 
* Lisäksi tietueessa on mpz_t -muuttuja saldo, joka on gmp.h -kirjaston oma muuttujatyyppi. Tämän 
* muuttujan sisältö luetaan myös tilitiedostosta.
*/

struct tili {
       char tilinumero[15];
       int tunnusluku;
       mpz_t saldo;
};

/*
* funktio, joka saa parametrinaan tilitietueen pointterin, josta funktiossa luetaan tilitiedoston 
* tiedot tili-tietueeseen. Funktio palauttaa toden, mikäli tiedoston molemmat rivit ovat ohjeiden 
* mukaiset. Jos tilitiedostoa ei ole, se on tyhjä tai muuten väärin tehty, palauttaa funktio falsen 
* virheilmoituksineen. 
*/

bool lueTilitiedot (struct tili *p) {
      char temp[40], rivi[256];
      FILE *file;

      strcat((strcpy(temp, p->tilinumero)), ".tili");
      if ((file = fopen(temp, "r")) != NULL) {
          if (fgets ( rivi, sizeof rivi, file ) != NULL) {
	      p->tunnusluku = atoi(rivi);
          } else {
	      printf("Tilitiedosto on epataydellinen!\n");
	      return false;
	  }
          if ( fgets ( rivi, sizeof rivi, file ) != NULL ) {
	      mpz_init_set_str(p->saldo, rivi, 10);
	      fclose(file);
	      return true;
          } else {
	      printf("Tilitiedosto on epataydellinen!\n");
	      return false;
	  }
      } else {
	  printf("Tilitiedostoa ei ole olemassa!\n");
	  return false;
      }
      
}

/*
* main():ssa joko nosto- tai talletustoiminnon jälkeen kutsuttava funktio, joka saa parametrikseen 
* tilitietueen osoittimen. Se aukaisee tilitiedoston kirjoitusta varten mikäli sellainen on olemassa 
* ja kirjoittaa tili-tietueessa olevat tiedot tiedostoon. Tämän jälkeen funktio tulostaa transaktion 
* jälkeisen saldon tietueesta.
*/ 

void kirjoitaTilitiedot (struct tili *p) {
      char temp[40], temp_saldo[256], temp_tunnusluku[15];
      FILE *file;

     strcat((strcpy(temp, p->tilinumero)), ".tili");
     if ((file = fopen(temp, "w")) == NULL) {
	fprintf(stderr, "Tilitiedostoa ei voida avata!\n");
     } else {
	 sprintf(temp_tunnusluku, "%d", p->tunnusluku);
	 fwrite(temp_tunnusluku, sizeof(temp_tunnusluku), 1, file);
	 fprintf(file, "\n");
	 mpz_get_str(temp_saldo, 10, p->saldo);
	 fwrite(temp_saldo, sizeof(temp_saldo), 1, file);
	 fclose(file);
     }
     gmp_printf("Saldosi on: %Zd\n", p->saldo);
}

/*
* muutamassa paikassa käytetty apufunktio, joka ottaa jonkin syötteen parametrikseen ja 
* tutkii koostuuko annettu syöte pelkästään numeroista. Jos ei, niin antaa virheilmoituksen 
* ja palauttaa falsen, ja jos koostuu niin palauttaa truen.
*/

bool onkoKokonaisluku (char * syote) {
    int i;
    bool kaikki_numeroita = 1;

    for (i = 0; i < strlen(syote); i++) {
        if (!isdigit(syote[i])) {
	   kaikki_numeroita = 0;
	   printf("Voit syottaa vain numeroita!\n");
	   break;
	}
    }
    return kaikki_numeroita;
}

/*
* funktio, joka aloittaa talletustoiminnon ja ottaa parametrikseen tilitietueen pointterin. 
* Funktiossa on looppi, joka pyörii niin kauan että käyttäjä antaa pelkästään numeroita 
* sisältävän syötteen, joka toimii talletettavana summana. Tämän jälkeen annettu summa lisätään 
* gmp.h -kirjaston omalla mpz_add -funktiolla tilitietueen saldoriville.
*/

void talleta (struct tili *p) {
     mpz_t talletus;
     mpz_init(talletus);
     char str_talletus[256];
     do {
	 printf("Paljonko haluat tallettaa? > ");
	 scanf("%s", str_talletus);
     } while (!onkoKokonaisluku(str_talletus));
     mpz_set_str(talletus,str_talletus,10);
     mpz_add(p->saldo,p->saldo,talletus);
}

/*
* funktio, joka saa parametrikseen tilitietueen pointterin. Tässäkin pyörii ensin looppi 
* joka kysyy käyttäjältä tilin tunnuslukua, ja looppi pyörii niin kauan että syöte sisältää 
* vain numeroita. Sen jälkeen testataan, vastaako annettu tunnusluku tilitietueessa olevaa 
* tunnuslukua ja jos vastaa, palauttaa funktio truen ja jos ei vastaa, palauttaa se falsen.
*/

bool tarkistaTunnusluku (struct tili *p) {
     char annettuTunnusluku[15];

     do {
	printf("Anna tunnuslukusi > ");
	scanf("%s", annettuTunnusluku);
     } while (!onkoKokonaisluku(annettuTunnusluku));
     
     if (atoi(annettuTunnusluku) == (int)p->tunnusluku) {
	return true;
     } else {
	printf("Tunnusluku vaarin\n");
	return false;
     }
}

/*
* funktio joka saa parametrikseen tilitietueen pointterin ja aloittaa nostotoiminnon. 
* Funktiossa on looppi, joka pyörii niin kauan että käyttäjä antaa pelkästään numeroita 
* sisältävän syötteen, joka toimii nostettavana summana. Tämän jälkeen käytetään gmp.h -kirjaston 
* omaa vertailumetodia mpz_cmp. Jos nostettava summa on pienempi kuin tilillä oleva summa, 
* miinustetaan se tietueessa olevasta saldomuuttujasta. Jos nostettava summa on isompi kuin 
* tilin saldo, annetaan virheilmoitus.
*/

void nosta (struct tili *p) {
     mpz_t nosto;
     mpz_init(nosto);
     char str_nosto[256];
     do {
	 printf("Paljonko haluat nostaa? > ");
	 scanf("%s", str_nosto);
     } while (!onkoKokonaisluku(str_nosto));
     mpz_set_str(nosto,str_nosto,10);
     if (mpz_cmp(p->saldo, nosto) > 0)  {
	 mpz_sub(p->saldo, p->saldo, nosto);
     } else {
	 printf("Et voi nostaa tililtasi enempaa kuin siella on!\n");
     }
}

/*
* pääohjelmassa main() pyöritetään do-while-looppeja niin kauan että käyttäjä valitsee 
* valikossa lopetuksen. Aluksi muodostetaan tilitietueesta pointteri tiliPtr, jolle varataan
* tarpeeksi tilaa. Sen jälkeen kysytään tilinumero jonka koostuminen vain numeroista tarkistetaan 
* onkoKokonaisluku():lla, jonka jälkeen kutsutaan lueTilitiedot() -funktiota annetulla syötteellä
* ja se joko onnistuu tai ei onnistu. Jos ei, niin kysytään tilinumeroa uudestaan. Jos onnistuu,
* tulostetaan valikko jossa käyttäjä voi valita saldokyselyn, talletuksen, noston tai lopetuksen. 
* Valinnat annetaan vastaavina numeroina jotka myös tarkistetaan virheiden varalta. Sitten 
* siirretään valinta switch caseen joista kutsutaan toimintojen edellyttämiä funktioita jotka 
* ovat esiteltyinä yläpuolella. Lopetuksen yhteydessä vapautetaan tietueen pointterille varattu 
* tila ja suljetaan ohjelma.
*/

int main() {
    char valinta[3];

    do {
	struct tili *tiliPtr;
	tiliPtr = (struct tili *)malloc(sizeof(struct tili));

	do {
	    printf("Tervetuloa!\n");
	    do {
		printf("Anna tilinumerosi kokonaislukuna > ");
		scanf("%s", tiliPtr->tilinumero);
	    } while (!onkoKokonaisluku(tiliPtr->tilinumero));
	} while (!lueTilitiedot(tiliPtr));
	    
	    printf("Kaytossasi on seuraavat valinnat:\n");
	    printf("[1] Saldo\n");
	    printf("[2] Talleta rahaa\n");
	    printf("[3] Nosta rahaa\n");
	    printf("[0] Lopeta\n");
	    do {
	    printf("Anna vaihtoehto > ");
	    scanf("%s", valinta);
	    } while (!onkoKokonaisluku(valinta));

	    switch (atoi(valinta)) {
		case 1:
		    if (tarkistaTunnusluku(tiliPtr)) {
		       gmp_printf("Saldosi on: %Zd\n", tiliPtr->saldo);
		    }
		    break;
		case 2:
		    if (tarkistaTunnusluku(tiliPtr)) {
		       talleta(tiliPtr);
		       kirjoitaTilitiedot(tiliPtr);
		    }
		    break;
		case 3:
		    if (tarkistaTunnusluku(tiliPtr)) {
		       nosta(tiliPtr);
		       kirjoitaTilitiedot(tiliPtr);
		    }
		    break;
		case 0:
		    free(tiliPtr);
		    exit(0);
		default:
		    break;
    
	    }
    printf("Nakemiin!\n\n");
    free(tiliPtr);
    } while (atoi(valinta) != 0);
}