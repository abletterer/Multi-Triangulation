 #ifndef __NODE_H__
 #define __NODE_H__

/*
 * Classe définissant les différents noeuds de l'arbre des maillages progressifs
 */
class Node {
    public:
        /*Liens dans l'arborescence*/
        Node* m_parent;
        Node* m_child_1;
        Node* m_child_2;
        
        /*Informations pour la transformation*/
        Dart m_dart;        //Brin courant
        Dart m_left_dart;   //Brin gauche du brin courant
        Dart m_right_dart;  //Brin droit du brin courant
    private:
    protected:
};

#endif
