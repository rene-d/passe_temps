// vim:set ts=4 sw=4 et:
//
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <list>
#include <boost/chrono.hpp>
#include <boost/format.hpp>
#include <unistd.h>


class passe_temps
{
    typedef boost::chrono::high_resolution_clock hr_clock;

    struct element
    {
        void    *ctx;
        const char *nom;
        size_t  calls;
        hr_clock::duration elapsed;
        hr_clock::time_point start;
    };

    // exercice: remplacer la list par une map
    typedef std::list<element>  liste;

    void        *ctx_;
    const char  *nom_;
    
    liste::iterator    last_active_;       // la mesure active précédente
    liste::iterator    current_active_;    // ce qu'on mesure

    static liste stack_;                    // la mesure active
    static liste::iterator active_;         // toutes les mesures

public:
    passe_temps(const char *nom, void *ctx)
    {
        hr_clock::time_point now = hr_clock::now();

        nom_ = nom;
        ctx_ = ctx;
        last_active_ = active_;

        if (active_ != stack_.end())
        {
            //std::cout << "suspend " << active_->nom << std::endl;

            active_->elapsed += now - active_->start;
            active_->start = hr_clock::time_point::min();
        }

        for (active_ = stack_.begin(); active_ != stack_.end(); ++active_)
        {
            if (active_->ctx == ctx_ && active_->nom == nom_)
            {
                assert(active_->start == hr_clock::time_point::min());
                //std::cout << "restart " << active_->nom << std::endl;

                ++active_->calls;
                active_->start = now;

                current_active_ = active_;
                return;
            }
        }

        element el;
        el.nom = nom;
        el.ctx = ctx;
        el.start = now;
        el.calls = 1;
        el.elapsed = hr_clock::duration::zero();

        stack_.push_back(el);
        active_ = stack_.end();
        --active_;

        current_active_ = active_;

        //std::cout << "start " << current_active_->nom << std::endl;
    }


    ~passe_temps()
    {
        hr_clock::time_point now = hr_clock::now();

        current_active_->elapsed += now - current_active_->start;
        current_active_->start = hr_clock::time_point::min();

        nom_ = 0;
        ctx_ = 0;

        //std::cout << "finish " << current_active_->nom << std::endl;

        active_ = last_active_;

        if (active_ != stack_.end())
        {
            //std::cout << "resume " << active_->nom << std::endl;
            active_->start = now;
        }
    }


    static void print()
    {
        std::cout << std::endl;
        std::cout << "mesures:" << std::endl;
        for (auto& i : stack_)
        {
            double d = i.elapsed.count() * ((double)hr_clock::period::num / hr_clock::period::den);
            std::cout << boost::format("  %-6s: %4s call(s)  %8.6f s   %7.3f ms/call") % i.nom % i.calls % d % (d / i.calls * 1000.) << std::endl;
        }
    }
};

passe_temps::liste passe_temps::stack_;
passe_temps::liste::iterator passe_temps::active_ = passe_temps::stack_.end();



// simulation imbriquée
//
class mikado
{
    int n = 0;
    int c1 = 0, c2 = 0, c3 = 0;

public:
    mikado()
    {
        run("A");

        std::cout << std::endl;
        std::cout << "stats d'appel:" << std::endl;
        std::cout << "  run = " << c1 << std::endl;
        std::cout << "  f1  = " << c2 << std::endl;
        std::cout << "  f2  = " << c3 << std::endl;
    }

private:
    void attente(int& c, int duree) const
    {
        ++c;

        // 10 ms
        usleep(1000 * duree);
    }

    void run(const char *nom)
    {
        passe_temps mm("run", this);

        std::cout << std::string(n * 2, ' ') << " run :";

        ++n;

        attente(c1);

        if (strpbrk(nom, "ABC"))
        {
            passe_temps mm("f1", this);
            f1(nom);
        }
        else
        {
            passe_temps mm("f2", this);
            f2(nom);
        }

        --n;
    }


    void f1(const char *nom)
    {
        std::cout << " f1 " << nom << std::endl;
        attente(c2, 13);

        if (n >= 9) return;

        if (!strcmp(nom, "A")) run("B");
        run("y");
        if (!strcmp(nom, "B")) run("C");
        run("z");
        if (!strcmp(nom, "C")) run("D");
    }


    void f2(const char *nom)
    {
        std::cout << " f2 " << nom << std::endl;
        attente(c3, 17);

        if (strpbrk(nom, "xyz")) return;

        run("A");
    }
};


void imbrique()
{
    mikado   m;
}


void simple2()
{
    // crée un autre point de mesure, le précédent est suspendu
    passe_temps m("100ms", NULL);

    usleep(100000);
}

void simple()
{
    // crée un point de mesure: on mesure le temps passé dans la fonction
    // jusqu'à en sortir, en excluant les autres fonctions chronométrées
    passe_temps m("500ms", NULL);

    usleep(500000);
    simple2();
}


int main()
{
    simple();
    imbrique();

    // résultats
    passe_temps::print();

    return 0;
}

