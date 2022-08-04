# Les différents types de commande d'accessoires

Les centrales DCC possède plusieurs types de commande d'accessoires, la plus couramant utilisée est la commande T (pour turnout).

## La commande turnout (dite T)
La commande "T" est très souvent utilisée pour commander les aiguillages de façon indépendante des autres accessoires. C'est-à-dire que la centrale ne considére pas les aiguillages comme de simples accessoires et qu'elle utilise une commande différente pour les autres types d'accessoires (feux de signalisation par exemple).

## La commande d'accessoires (dite A)
La commande "A" quant à elle est plus générale et permet de commander n'importe quelle sorte d'accessoires sur votre réseau. Elle comporte néanmoins des limitations techniques dues à sa conception ainsi qu'une complexité d'adaptation accrue des décodeurs d'accessoires du fait qu'elle utilise une adresse ET une sous-adresse.

## La commande d'accessoires étendu (dite X)
La commande "X" dite "accessoires étendu" est la plus simple et la plus flexible. Elle utilise une seule adresse et possède une plage d'état allant jusqu'à 255 par commande (contre seulement 2 pour les commandes A et T). Elle sera donc tout adaptée si vous possédez une variété d'accessoires très large (comme des feux de signalisation complexe) sur votre réseau.

## :white_check_mark: Conclusion
Nous conseillons donc d'utiliser la commande X sur votre réseau afin de bénéficier de la plus grande flexibilité possible dans la mesure où votre centrale est compatible.

La centrale DCCpp_S88 disponible à [cette adresse](https://github.com/MathieuAndrade/DCCpp_S88)
  est la plus évoluée (au moment où ces lignes sont écrites) et est maintenue à jour par plusieurs personnes, nous conseillons donc de l'utiliser afin d'avoir le plus de comptabilité possible avec vos logiciels (les logiciels CDM-Rail et JMRI sont pleinement compatibles avec celle-ci).

## :warning: Points à prendre en compte
Attention il peut arriver dans certaines centrales que la commande T désactive complètement la commande d'accessoire simple.

Comme la commande "T" est la plus couramment utilisée, le logiciel CDM-Rail utilisera cette commande par défaut, mais cela implique la désactivation complète des commandes d'accessoires. CDM-Rail sera donc incapable de piloter d'autres accessoires (comme des feux de signalisation) que des aiguillages si vous décidez d'utiliser la commande "T".

