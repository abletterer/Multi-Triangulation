 #ifndef __NODE_H__
 #define __NODE_H__

/*
 * Classe définissant les différents noeuds de l'arbre des maillages progressifs
 */
class Node {
    public:
        Node(VSplit& vsplit);
        ~Node();
    private:
        /*Liens dans l'arborescence*/
        Node* m_parent;
        Node* m_child_1;
        Node* m_child_2;
        
        /*Informations pour la transformation*/
        VSplit& m_vsplit
    protected:
};

#endif
