## MultiSeg - Hierarchical segmentation algorithm for radar and optical data

### What is MultiSeg?

MultiSeg consists on a specialized segmentation technique devised for SAR (Synthetic Aperture Radar) and optical imagery. Initially, images are compressed at different rates creating an image pyramid, then a region growing procedure is used in combination with a split and merge technique at the different compression levels. In sequence, the program processes the image pyramid from the coarser to the finer compression levels, applying a border refinement heuristic each time it changes from one level to the next.

Created in the C++ language, with the support from the open-source library [TerraLib](http://www.dpi.inpe.br/terralib5/wiki/doku.php).
The devised software architecture permits easy extension of its capabilities.

### Methodology

#### Processing Chain
![](images/methodology.png)
#### Example
![](images/methodology-example.png)

### References

UBA, D. M.; DUTRA, L. V.; SOARES, M. D.; COSTA, G. A. O. P. Implementation of a hierarchical segmentation algorithm for radar and optical data using TerraLib. In: SIMPÓSIO BRASILEIRO DE SENSORIAMENTO REMOTO, 17. (SBSR), 2015, João Pessoa. Anais... São José dos Campos: INPE, 2015. p. 4041-4048. Internet. ISBN 978-85-17-0076-8. Available from: [Link](http://urlib.net/rep/8JMKD3MGP6W34M/3JM4CC8).

COSTA, G.; DUTRA, L. V.; UBA, D. M.; SOARES, M.; FEITOSA, R.; ROSA, R. MultiSeg: a hierarchical segmentation algorithm for radar and optical data. In: INTERNATIONAL CARTOGRAPHIC CONFERENCE, 27., , Rio de Janeiro, RJ. 2015.

SOUSA JÚNIOR, M. A. Segmentação multi-níveis e multi-modelos para imagens de radar e ópticas. 2005. 131 p. (INPE-14466-TDI/1147). Tese (Doutorado em Computação Aplicada) - Instituto Nacional de Pesquisas Espaciais (INPE), São José dos Campos, 2005. Available from: [Link](http://urlib.net/rep/6qtX3pFwXQZ3P8SECKy/Gk4Ky).
