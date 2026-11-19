// Spanzuratoarea - proiect 5.1 (varianta extinsa)
// Conversie WinBGIm -> SDL2. Logica jocului si meniurile sunt pastrate.
// Dependinte: SDL2, SDL2_image, SDL2_ttf.

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <fstream>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <string>
using namespace std;

const int MAX_GRESELI = 10;
string numeCategorie[5] = {
    "Animale",
    "Tari",
    "Fructe",
    "Meserii",
    "Amestecat"
};

int categorieAleasa = 4;

// Culori echivalente cu cele folosite in WinBGIm
const int BLACK = 0;
const int BLUE = 1;
const int GREEN = 2;
const int RED = 3;
const int BROWN = 4;
const int DARKGRAY = 5;
const int WHITE = 6;

const int LATIME = 640;
const int INALTIME = 480;

SDL_Window* fereastra = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* imagineFundal = nullptr;
bool ruleazaAplicatia = true;

SDL_Color culoareSDL(int culoare)
{
    switch (culoare)
    {
        case BLUE:     return {0, 0, 255, 255};
        case GREEN:    return {0, 180, 0, 255};
        case RED:      return {220, 0, 0, 255};
        case BROWN:    return {120, 70, 25, 255};
        case DARKGRAY: return {90, 90, 90, 255};
        case WHITE:    return {255, 255, 255, 255};
        default:       return {0, 0, 0, 255};
    }
}

void seteazaCuloare(int culoare)
{
    SDL_Color c = culoareSDL(culoare);
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
}

TTF_Font* deschideFont(int marime)
{
    const char* fonturi[] = {
        "arial.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
    };

    for (const char* cale : fonturi)
    {
        TTF_Font* f = TTF_OpenFont(cale, marime * 10);
        if (f) return f;
    }
    return nullptr;
}

// Echivalent SDL pentru getch() din WinBGIm: asteapta o tasta si intoarce cod ASCII.
int getch()
{
    SDL_Event eveniment;
    while (SDL_WaitEvent(&eveniment))
    {
        if (eveniment.type == SDL_QUIT)
        {
            ruleazaAplicatia = false;
            return 27;
        }

        if (eveniment.type == SDL_KEYDOWN)
        {
            SDL_Keycode tasta = eveniment.key.keysym.sym;

            if (tasta == SDLK_ESCAPE) return 27;
            if (tasta >= SDLK_a && tasta <= SDLK_z)
                return 'a' + (int)(tasta - SDLK_a);
            if (tasta >= SDLK_0 && tasta <= SDLK_9)
                return '0' + (int)(tasta - SDLK_0);
            if (tasta == SDLK_SPACE) return ' ';
            if (tasta == SDLK_RETURN) return '\n';
        }
    }
    ruleazaAplicatia = false;
    return 27;
}

// scrie un text la pozitia x,y
void scrie(int x, int y, string text, int marime, int culoare)
{
    if (text.empty()) return;

    TTF_Font* font = deschideFont(marime);
    if (!font) return;

    SDL_Color c = culoareSDL(culoare);
    SDL_Surface* suprafata = TTF_RenderUTF8_Blended(font, text.c_str(), c);
    if (suprafata)
    {
        SDL_Texture* textura = SDL_CreateTextureFromSurface(renderer, suprafata);
        if (textura)
        {
            SDL_Rect destinatie = {x, y, suprafata->w, suprafata->h};
            SDL_RenderCopy(renderer, textura, nullptr, &destinatie);
            SDL_DestroyTexture(textura);
        }
        SDL_FreeSurface(suprafata);
    }
    TTF_CloseFont(font);
}

// scrie un text pe mijlocul ferestrei (fereastra are latimea 640)
void scrieCentrat(int y, string text, int marime, int culoare)
{
    TTF_Font* font = deschideFont(marime);
    if (!font)
    {
        scrie(0, y, text, marime, culoare);
        return;
    }

    int latimeText = 0;
    TTF_SizeUTF8(font, text.c_str(), &latimeText, nullptr);
    TTF_CloseFont(font);

    int x = (LATIME - latimeText) / 2;
    scrie(x, y, text, marime, culoare);
}

// deseneaza un dreptunghi alb cu margine neagra
void panou(int x1, int y1, int x2, int y2)
{
    SDL_Rect dreptunghi = {x1, y1, x2 - x1, y2 - y1};

    seteazaCuloare(WHITE);
    SDL_RenderFillRect(renderer, &dreptunghi);

    seteazaCuloare(BLACK);
    SDL_RenderDrawRect(renderer, &dreptunghi);
}

// sterge ecranul si pune imaginea de fundal
void deseneazaFundal()
{
    SDL_SetRenderDrawColor(renderer, 220, 235, 245, 255);
    SDL_RenderClear(renderer);

    if (imagineFundal)
    {
        SDL_Rect destinatie = {0, 0, 639, 479};
        SDL_RenderCopy(renderer, imagineFundal, nullptr, &destinatie);
    }
}

void linieGroasa(int x1, int y1, int x2, int y2, int grosime)
{
    for (int i = 0; i < grosime; ++i)
        SDL_RenderDrawLine(renderer, x1, y1 + i, x2, y2 + i);
}

void cerc(int cx, int cy, int raza)
{
    int x = raza;
    int y = 0;
    int eroare = 0;

    while (x >= y)
    {
        SDL_RenderDrawPoint(renderer, cx + x, cy + y);
        SDL_RenderDrawPoint(renderer, cx + y, cy + x);
        SDL_RenderDrawPoint(renderer, cx - y, cy + x);
        SDL_RenderDrawPoint(renderer, cx - x, cy + y);
        SDL_RenderDrawPoint(renderer, cx - x, cy - y);
        SDL_RenderDrawPoint(renderer, cx - y, cy - x);
        SDL_RenderDrawPoint(renderer, cx + y, cy - x);
        SDL_RenderDrawPoint(renderer, cx + x, cy - y);

        if (eroare <= 0) { ++y; eroare += 2 * y + 1; }
        if (eroare > 0) { --x; eroare -= 2 * x + 1; }
    }
}

// deseneaza spanzuratoarea in functie de numarul de greseli
// 1-4 = ce e din lemn, 5-10 = omul
void deseneazaSpanzuratoare(int greseli)
{
    seteazaCuloare(BROWN);
    if (greseli >= 1) linieGroasa(40, 430, 280, 430, 4);     // baza
    if (greseli >= 2) linieGroasa(90, 430, 90, 110, 4);     // stalpul
    if (greseli >= 3) linieGroasa(90, 110, 210, 110, 4);    // bara de sus
    if (greseli >= 4) linieGroasa(210, 110, 210, 150, 4);   // streangul

    seteazaCuloare(BLACK);
    if (greseli >= 5)  cerc(210, 170, 20);                  // capul
    if (greseli >= 6)  SDL_RenderDrawLine(renderer, 210, 190, 210, 280); // corpul
    if (greseli >= 7)  SDL_RenderDrawLine(renderer, 210, 205, 175, 250); // mana stanga
    if (greseli >= 8)  SDL_RenderDrawLine(renderer, 210, 205, 245, 250); // mana dreapta
    if (greseli >= 9)  SDL_RenderDrawLine(renderer, 210, 280, 180, 340); // piciorul stang
    if (greseli >= 10) SDL_RenderDrawLine(renderer, 210, 280, 240, 340); // piciorul drept
}

// ------------------------------------------------------------
// Functii pentru fisiere si cuvinte
// ------------------------------------------------------------

// alege un cuvant la intamplare din categoria data
string alegeCuvant(int categorie)
{
    string cuvant = "";
    if (categorie == 4)
        categorie = rand() % 4;

    string fisiere[4] = {"animale.txt", "tari.txt", "fructe.txt", "meserii.txt"};
    ifstream fisier(fisiere[categorie].c_str());
    if (!fisier)
        return "";

    int numarDeCuvinte, numar;
    fisier >> numarDeCuvinte;
    numar = 1 + rand() % numarDeCuvinte;
    for (int i = 1; i <= numar; i++)
        fisier >> cuvant;
    fisier.close();
    return cuvant;
}

// ------------------------------------------------------------
// Ecranul jocului
// ------------------------------------------------------------

// deseneaza tot ecranul de joc (se apeleaza dupa fiecare litera)
void deseneazaJoc(string categorie, string ecran, string gresite, int nrGreseli,
                  string mesaj, int culoareMesaj, string jos)
{
    deseneazaFundal();

    // titlul
    panou(20, 10, 630, 50);
    scrieCentrat(20, "Categoria: " + categorie, 2, BLUE);

    // panoul din stanga: spanzuratoarea
    panou(20, 60, 300, 450);
    deseneazaSpanzuratoare(nrGreseli);

    // panoul din dreapta: cuvantul si informatiile
    panou(310, 60, 630, 450);

    // punem spatii intre litere ca sa se vada mai bine
    string afisare = "";
    for (int i = 0; i < (int)ecran.length(); i++)
    {
        afisare = afisare + ecran[i];
        afisare = afisare + " ";
    }
    scrie(318, 80, "Cuvantul:", 2, BLACK);
    scrie(318, 115, afisare, 2, BLUE);

    scrie(318, 175, "Greseli: " + to_string(nrGreseli) + " / " + to_string(MAX_GRESELI), 2, RED);

    scrie(318, 225, "Litere gresite:", 2, BLACK);
    string gresiteSpatiate = "";
    for (int i = 0; i < (int)gresite.length(); i++)
    {
        gresiteSpatiate = gresiteSpatiate + (char)toupper(gresite[i]);
        gresiteSpatiate = gresiteSpatiate + " ";
    }
    scrie(318, 255, gresiteSpatiate, 2, RED);

    scrie(318, 320, mesaj, 2, culoareMesaj);
    scrie(318, 365, jos, 1, BLACK);
    scrie(318, 425, "ESC = inapoi la meniu", 1, DARKGRAY);

    SDL_RenderPresent(renderer);
}

// jocul propriu-zis (pentru o singura runda)
void joaca(int categorie)
{
    string cuvant = alegeCuvant(categorie);
    if (cuvant == "")
    {
        deseneazaFundal();
        panou(100, 180, 540, 300);
        scrieCentrat(200, "Nu am gasit fisierul", 2, RED);
        scrieCentrat(230, "cu cuvinte!", 2, RED);
        scrieCentrat(265, "Apasa o tasta...", 1, BLACK);
        SDL_RenderPresent(renderer);
        getch();
        return;
    }

    // numele categoriei afisate (daca e Amestecat, il scriem tot asa)
    string numeCategorie[5] = {"Animale", "Tari", "Fructe", "Meserii", "Amestecat"};
    string numeAfisat = numeCategorie[categorie];

    int lung = cuvant.length();
    string ecran = cuvant;     // cuvantul care se vede pe ecran
    string folosite = "";      // toate literele incercate pana acum
    string gresite = "";       // doar literele care nu sunt in cuvant
    int i;

    // se afiseaza prima si ultima litera, restul sunt '_'
    // (si daca prima/ultima litera mai apare in cuvant, o aratam si acolo)
    for (i = 0; i < lung; i++)
    {
        if (cuvant[i] == cuvant[0] || cuvant[i] == cuvant[lung - 1])
            ecran[i] = cuvant[i];
        else
            ecran[i] = '_';
    }
    folosite = folosite + cuvant[0];
    folosite = folosite + cuvant[lung - 1];

    int nrGreseli = 0;
    char litera;
    bool gasita;
    string mesaj = "Alege o litera!";
    int culoareMesaj = BLACK;

    while (nrGreseli < MAX_GRESELI && ecran != cuvant && ruleazaAplicatia)
    {
        deseneazaJoc(numeAfisat, ecran, gresite, nrGreseli, mesaj, culoareMesaj,
                     "Apasa o litera de la tastatura");

        litera = getch();
        if (!ruleazaAplicatia || litera == 27)
            return;
        litera = tolower(litera);

        if (litera < 'a' || litera > 'z')
        {
            mesaj = "Doar litere!";
            culoareMesaj = RED;
        }
        else if (folosite.find(litera) != string::npos)
        {
            mesaj = "Deja incercata!";
            culoareMesaj = RED;
        }
        else
        {
            folosite = folosite + litera;
            gasita = false;
            for (i = 0; i < lung; i++)
                if (cuvant[i] == litera)
                {
                    gasita = true;
                    ecran[i] = litera;
                }

            if (gasita)
            {
                mesaj = "Bravo, exista!";
                culoareMesaj = GREEN;
            }
            else
            {
                mesaj = "Nu exista!";
                culoareMesaj = RED;
                nrGreseli++;
                gresite = gresite + litera;
            }
        }
    }

    // jocul s-a terminat
    if (ecran == cuvant)
        deseneazaJoc(numeAfisat, ecran, gresite, nrGreseli, "AI CASTIGAT!", GREEN,
                     "Apasa o tasta pentru meniu");
    else
        deseneazaJoc(numeAfisat, cuvant, gresite, nrGreseli, "AI PIERDUT!", RED,
                     "Cuvantul era: " + cuvant);
    getch();
}

// ------------------------------------------------------------
// Meniurile
// ------------------------------------------------------------

void deseneazaMeniu()
{
    deseneazaFundal();
    panou(110, 70, 530, 410);
    scrieCentrat(95, "SPANZURATOAREA", 3, BLUE);
    scrieCentrat(130, "******************", 2, BLUE);

    scrie(160, 190, "1. Joc nou", 2, BLACK);
    scrie(160, 235, "2. Alege categoria", 2, BLACK);
    scrie(160, 280, "3. Cum se joaca", 2, BLACK);
    scrie(160, 325, "0. Iesire", 2, BLACK);

    scrieCentrat(375, "Categoria: " + numeCategorie[categorieAleasa], 1, RED);
    SDL_RenderPresent(renderer);
}

void alegeCategoria()
{
    int tasta = 0;
    while ((tasta < '1' || tasta > '5') && ruleazaAplicatia)
    {
        deseneazaFundal();
        panou(110, 70, 530, 410);
        scrieCentrat(90, "Alege categoria", 3, BLUE);
        scrie(160, 160, "1. Animale", 2, BLACK);
        scrie(160, 200, "2. Tari", 2, BLACK);
        scrie(160, 240, "3. Fructe", 2, BLACK);
        scrie(160, 280, "4. Meserii", 2, BLACK);
        scrie(160, 320, "5. Amestecat", 2, BLACK);
        scrieCentrat(375, "Apasa tasta cu numarul categoriei", 1, DARKGRAY);
        SDL_RenderPresent(renderer);
        tasta = getch();
    }
    if (tasta >= '1' && tasta <= '5')
        categorieAleasa = tasta - '1';
}

void cumSeJoaca()
{
    deseneazaFundal();
    panou(40, 70, 600, 410);
    scrieCentrat(90, "Cum se joaca", 3, BLUE);
    scrie(60, 150, "Trebuie sa ghicesti cuvantul.", 2, BLACK);
    scrie(60, 185, "Prima si ultima litera sunt", 2, BLACK);
    scrie(60, 210, "date. Apasa litere pe tastatura.", 2, BLACK);
    scrie(60, 255, "Daca litera e in cuvant, apare.", 2, BLACK);
    scrie(60, 290, "Daca nu, se deseneaza o parte", 2, BLACK);
    scrie(60, 315, "din spanzuratoare.", 2, BLACK);
    scrie(60, 355, "Ai voie la 10 greseli.", 2, RED);
    scrieCentrat(385, "Apasa o tasta pentru a reveni", 1, DARKGRAY);
    SDL_RenderPresent(renderer);
    getch();
}

// ------------------------------------------------------------
// Initializare si inchidere SDL2
// ------------------------------------------------------------

bool initializeazaSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return false;

    if ((IMG_Init(IMG_INIT_JPG) & IMG_INIT_JPG) == 0)
        SDL_Log("SDL_image: %s", IMG_GetError());

    if (TTF_Init() != 0)
        return false;

    fereastra = SDL_CreateWindow("Spanzuratoarea",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        LATIME, INALTIME, SDL_WINDOW_SHOWN);

    if (!fereastra)
        return false;

    renderer = SDL_CreateRenderer(fereastra, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!renderer)
        return false;

    SDL_Surface* suprafata = IMG_Load("fundal.jpg");
    if (suprafata)
    {
        imagineFundal = SDL_CreateTextureFromSurface(renderer, suprafata);
        SDL_FreeSurface(suprafata);
    }
    else
        SDL_Log("Nu s-a putut incarca fundal.jpg: %s", IMG_GetError());

    // Verifica existenta unui font, fara sa oblige folosirea unui anumit font.
    TTF_Font* testFont = deschideFont(2);
    if (!testFont)
    {
        SDL_Log("Nu gasesc arial.ttf sau DejaVuSans.ttf. Pune un fisier .ttf langa program.");
        return false;
    }
    TTF_CloseFont(testFont);

    return true;
}

void inchideSDL()
{
    if (imagineFundal) SDL_DestroyTexture(imagineFundal);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (fereastra) SDL_DestroyWindow(fereastra);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

// ------------------------------------------------------------
// Programul principal
// ------------------------------------------------------------

int main()
{
    srand(time(NULL));

    if (!initializeazaSDL())
    {
        SDL_Log("Eroare la initializarea SDL: %s", SDL_GetError());
        inchideSDL();
        return 1;
    }

    int optiune = 0;
    while (optiune != '0' && ruleazaAplicatia)
    {
        deseneazaMeniu();
        optiune = getch();

        if (!ruleazaAplicatia) break;

        if (optiune == '1')
            joaca(categorieAleasa);
        else if (optiune == '2')
            alegeCategoria();
        else if (optiune == '3')
            cumSeJoaca();
    }

    inchideSDL();
    return 0;
}
