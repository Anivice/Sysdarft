/***
 *
 *                                                                             dddddddd
 *       SSSSSSSSSSSSSSS                                                       d::::::d                                          ffffffffffffffff           tttt
 *     SS:::::::::::::::S                                                      d::::::d                                         f::::::::::::::::f       ttt:::t
 *    S:::::SSSSSS::::::S                                                      d::::::d                                        f::::::::::::::::::f      t:::::t
 *    S:::::S     SSSSSSS                                                      d:::::d                                         f::::::fffffff:::::f      t:::::t
 *    S:::::S            yyyyyyy           yyyyyyy    ssssssssss       ddddddddd:::::d   aaaaaaaaaaaaa   rrrrr   rrrrrrrrr     f:::::f       ffffffttttttt:::::ttttttt
 *    S:::::S             y:::::y         y:::::y   ss::::::::::s    dd::::::::::::::d   a::::::::::::a  r::::rrr:::::::::r    f:::::f             t:::::::::::::::::t
 *     S::::SSSS           y:::::y       y:::::y  ss:::::::::::::s  d::::::::::::::::d   aaaaaaaaa:::::a r:::::::::::::::::r  f:::::::ffffff       t:::::::::::::::::t
 *      SS::::::SSSSS       y:::::y     y:::::y   s::::::ssss:::::sd:::::::ddddd:::::d            a::::a rr::::::rrrrr::::::r f::::::::::::f       tttttt:::::::tttttt
 *        SSS::::::::SS      y:::::y   y:::::y     s:::::s  ssssss d::::::d    d:::::d     aaaaaaa:::::a  r:::::r     r:::::r f::::::::::::f             t:::::t
 *           SSSSSS::::S      y:::::y y:::::y        s::::::s      d:::::d     d:::::d   aa::::::::::::a  r:::::r     rrrrrrr f:::::::ffffff             t:::::t
 *                S:::::S      y:::::y:::::y            s::::::s   d:::::d     d:::::d  a::::aaaa::::::a  r:::::r              f:::::f                   t:::::t
 *                S:::::S       y:::::::::y       ssssss   s:::::s d:::::d     d:::::d a::::a    a:::::a  r:::::r              f:::::f                   t:::::t    tttttt
 *    SSSSSSS     S:::::S        y:::::::y        s:::::ssss::::::sd::::::ddddd::::::dda::::a    a:::::a  r:::::r             f:::::::f                  t::::::tttt:::::t
 *    S::::::SSSSSS:::::S         y:::::y         s::::::::::::::s  d:::::::::::::::::da:::::aaaa::::::a  r:::::r             f:::::::f                  tt::::::::::::::t
 *    S:::::::::::::::SS         y:::::y           s:::::::::::ss    d:::::::::ddd::::d a::::::::::aa:::a r:::::r             f:::::::f                    tt:::::::::::tt
 *     SSSSSSSSSSSSSSS          y:::::y             sssssssssss       ddddddddd   ddddd  aaaaaaaaaa  aaaa rrrrrrr             fffffffff                      ttttttttttt
 *                             y:::::y
 *                            y:::::y
 *                           y:::::y
 *                          y:::::y
 *                         yyyyyyy
 *
 *
 */

#include <stdio.h>

#define RED     "\x1b[31;1m"
#define GREEN   "\x1b[32;1m"
#define YELLOW  "\x1b[33;1m"
#define BLUE    "\x1b[34;1m"
#define WHITE   "\x1b[37;1m"
#define GREY    "\x1b[90;1m"
#define RESET   "\x1b[0m"

static constexpr char logo[] =
" *                                                                             dddddddd                                                                                        *\n"
" *       SSSSSSSSSSSSSSS                                                       d::::::d                                          ffffffffffffffff           tttt               *\n"
" *     SS:::::::::::::::S                                                      d::::::d                                         f::::::::::::::::f       ttt:::t               *\n"
" *    S:::::SSSSSS::::::S                                                      d::::::d                                        f::::::::::::::::::f      t:::::t               *\n"
" *    S:::::S     SSSSSSS                                                      d:::::d                                         f::::::fffffff:::::f      t:::::t               *\n"
" *    S:::::S            yyyyyyy           yyyyyyy    ssssssssss       ddddddddd:::::d   aaaaaaaaaaaaa   rrrrr   rrrrrrrrr     f:::::f       ffffffttttttt:::::ttttttt         *\n"
" *    S:::::S             y:::::y         y:::::y   ss::::::::::s    dd::::::::::::::d   a::::::::::::a  r::::rrr:::::::::r    f:::::f             t:::::::::::::::::t         *\n"
" *     S::::SSSS           y:::::y       y:::::y  ss:::::::::::::s  d::::::::::::::::d   aaaaaaaaa:::::a r:::::::::::::::::r  f:::::::ffffff       t:::::::::::::::::t         *\n"
" *      SS::::::SSSSS       y:::::y     y:::::y   s::::::ssss:::::sd:::::::ddddd:::::d            a::::a rr::::::rrrrr::::::r f::::::::::::f       tttttt:::::::tttttt         *\n"
" *        SSS::::::::SS      y:::::y   y:::::y     s:::::s  ssssss d::::::d    d:::::d     aaaaaaa:::::a  r:::::r     r:::::r f::::::::::::f             t:::::t               *\n"
" *           SSSSSS::::S      y:::::y y:::::y        s::::::s      d:::::d     d:::::d   aa::::::::::::a  r:::::r     rrrrrrr f:::::::ffffff             t:::::t               *\n"
" *                S:::::S      y:::::y:::::y            s::::::s   d:::::d     d:::::d  a::::aaaa::::::a  r:::::r              f:::::f                   t:::::t               *\n"
" *                S:::::S       y:::::::::y       ssssss   s:::::s d:::::d     d:::::d a::::a    a:::::a  r:::::r              f:::::f                   t:::::t    tttttt     *\n"
" *    SSSSSSS     S:::::S        y:::::::y        s:::::ssss::::::sd::::::ddddd::::::dda::::a    a:::::a  r:::::r             f:::::::f                  t::::::tttt:::::t     *\n"
" *    S::::::SSSSSS:::::S         y:::::y         s::::::::::::::s  d:::::::::::::::::da:::::aaaa::::::a  r:::::r             f:::::::f                  tt::::::::::::::t     *\n"
" *    S:::::::::::::::SS         y:::::y           s:::::::::::ss    d:::::::::ddd::::d a::::::::::aa:::a r:::::r             f:::::::f                    tt:::::::::::tt     *\n"
" *     SSSSSSSSSSSSSSS          y:::::y             sssssssssss       ddddddddd   ddddd  aaaaaaaaaa  aaaa rrrrrrr             fffffffff                      ttttttttttt       *\n"
" *                             y:::::y                                                                                                                                         *\n"
" *                            y:::::y                                                                                                                                          *\n"
" *                           y:::::y                                                                                                                                           *\n"
" *                          y:::::y                                                                                                                                            *\n"
" *                         yyyyyyy                                                                                                                                             *\n"
"\n\n";

void printLogo()
{
    for (unsigned int i = 0; i < sizeof(logo); i++)
    {
        if (logo[i] != '\n') {
            if (logo[i] == ' ' || logo[i] == '*') {
                printf(GREY "%%" RESET);
            } else if (logo[i] == ':') {
                printf(WHITE "%%" RESET);
            } else {
                printf(BLUE "%c" RESET, logo[i]);
            }
        } else {
            printf("%c", logo[i]);
        }
    }
}
