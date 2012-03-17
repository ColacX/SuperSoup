//imagine holding a snake by its head, the snake can have an infinte long body if there's infinite much memory

#pragma once

template <typename T> class SingleLinkedList{
protected:
    class Element{
    protected:
        T data;
        Element* link;

    public:
        Element(T data){
            this->data = data;
            this->link = 0;
        }
                
        T getData(){
            return this->data;
        }
        void setLink(Element* element){
            this->link = element;
        }
        Element* getLink(){
            return this->link;
        }
    };

    unsigned int elementCount;
    Element* elementHead;
    Element* elementTail;

public:
    SingleLinkedList(){
        this->elementCount = 0;
        this->elementHead = 0;
        this->elementTail = 0;
    }
    ~SingleLinkedList(){
        this->removeAll();
    }

    unsigned int getCount(){
        return this->elementCount;
    }

    void add(T data){
        Element* newElement = new Element(data);
            
        if(this->elementHead == 0 || this->elementTail == 0){
            this->elementHead = newElement;
            this->elementTail = newElement;
        }
        else{
            this->elementTail->setLink( newElement );
            this->elementTail = newElement;
        }
        this->elementCount++;
    }

    T get(unsigned int index){
        if( index+1 > elementCount || index == -1){
            throw "SingleLinkedList: element does not exist";
        }
        else{
            Element* temporary = this->elementHead;
            for(unsigned int i=0; i<index; i++){
                temporary = temporary->getLink();
            }
            return temporary->getData();
        }
    }

    void removeAll(){
        if(this->elementHead != 0){

            while(this->elementHead->getLink() != 0){
                Element* temporary = this->elementHead;

                while(temporary->getLink()->getLink() != 0){                    
                    temporary = temporary->getLink();
                }
                delete temporary->getLink();
                temporary->setLink(0);
            }

            delete this->elementHead;
            this->elementHead = 0;
            this->elementTail = 0;
            this->elementCount = 0;
        }
    }

    void insert(T data, unsigned int index){
        if(index >= this->elementCount){
            this->add(data);
        }
        else{
            Element* newElement = new Element(data);

            if( index == 0 ){
                Element* temporary = this->elementHead;
                this->elementHead = newElement;
                newElement->setLink(temporary);
            }
            else{
                Element* temporary = this->elementHead;
                unsigned int i = 0;
                while( index != i+1 ){
                    temporary = temporary->getLink();
                    i++;
                }
                Element* limbo = temporary->getLink();
                temporary->setLink(newElement);
                newElement->setLink(limbo);
            }

            this->elementCount++;
        }
    }

    void remove(unsigned int index){
        if( index >= this->elementCount ){
            //do nothing the element does not exist
        }
        else if( index == 0){
            Element* temporary = this->elementHead->getLink();
            delete this->elementHead;
            this->elementHead = temporary;
            this->elementCount--;
        }
        else{
            Element* temporary = this->elementHead;
                
            unsigned int i=1;
            while( i<index ){
                temporary = temporary->getLink();
                i++;
            }

            Element* limbo = temporary->getLink()->getLink();
            delete temporary->getLink();
            temporary->setLink(limbo);
            this->elementCount--;
        }
    }
};