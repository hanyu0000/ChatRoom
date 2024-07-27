#ifndef CIRCLE_H
#define CIRCLE_H

#include <iostream>
#include <cstring>
using namespace std;

class Base
{
public:
    Base()
    {
        _phead = new Node();
    }

    ~Base() // 析沟函数：释放链表中的所有节点，防止内存泄漏
    {
        Node *ptr = _phead;
        while (ptr != nullptr)
        {
            Node *next = ptr->getNext();
            delete ptr;
            ptr = next;
        }
    }

    bool CheckUserDate(char *name, char *pwd) // 判断用户名、密码是否匹配
    {
        Node *ptr = _phead->getNext();
        while (ptr != nullptr)
        {
            if ((strcmp(name, ptr->_name) == 0 && strcmp(pwd, ptr->_pwd) == 0))
                return true;
            ptr = ptr->getNext();
        }
        return false;
    }

    bool QueryName(char *name) // 名字是否匹配
    {
        Node *ptr = _phead->getNext();
        while (ptr != nullptr)
        {
            if (strcmp(name, ptr->_name) == 0)
                return true;
            ptr = ptr->getNext();
        }
        return false;
    }

    void WriteUserDate(char *name, char *pwd)
    {
        Node *p = new Node(name, pwd);
        p->setNext(_phead->getNext());
        _phead->setNext(p);
    }

    void DeleteUserDate(char *name, char *pwd)
    {
        Node *prev = _phead;
        Node *cur = prev->getNext();

        while (cur != nullptr)
        {
            if (strcmp(cur->_name, name) == 0 && strcmp(cur->_pwd, pwd) == 0)
            {
                prev->setNext(cur->getNext());
                delete cur;
                return;
            }
            prev = cur;
            cur = cur->getNext();
        }
    }

private:
    struct Node
    {
        Node() : _pnext(nullptr) {} // 构造函数

        Node(char *n, char *p) : _pnext(nullptr)
        {
            strcpy(_name, n);
            strcpy(_pwd, p);
        }

        char _name[50];
        char _pwd[50];
        Node *_pnext;

        void setNext(Node *p)
        {
            _pnext = p;
        }

        Node *getNext()
        {
            return _pnext;
        }
    };
    Node *_phead;
};
#endif