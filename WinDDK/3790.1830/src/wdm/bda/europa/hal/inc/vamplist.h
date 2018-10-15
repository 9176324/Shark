//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 1999
//  All rights are reserved. Reproduction in whole or in part is prohibited
//  without the written consent of the copyright owner.
//
//  Philips reserves the right to make changes without notice at any time.
//  Philips makes no warranty, expressed, implied or statutory, including but
//  not limited to any implied warranty of merchantability or fitness for any
//  particular purpose, or that the use will not infringe any third party
//  patent, copyright or trademark. Philips must not be liable for any loss
//  or damage arising from its use.
//
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//
// @doc      INTERNAL EXTERNAL
// @module   VampList | This module contains a list class, which elements are
//           linked together in both directions (forward/backward).<nl>
// @end
// Filename: VampList.h
//
// Routines/Functions:
//
//  public:
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////
#if !defined(_VAMPLIST_H_)
#define _VAMPLIST_H_

#undef TYPEID
#define TYPEID CVAMPBUFFER
//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampListEntry | The list class is borrowed based on the LIST_ENTRY
//         structure of NT, where the list node must be part of the object.
//         The class and the entry struct are rewritten for C++ use only. For
//         security the CVampListEntry should be derived as private and the
//         template class should be set as a friend class.
// @end
//
//////////////////////////////////////////////////////////////////////////////
class CVampListEntry
{
    // @access public 
public:
    // @cmember | CVampListEntry | () | Constructor. Initializes forward and
    //          backward links.
    CVampListEntry () : 
        Flink( NULL ), Blink( NULL ) {};

    // @cmember Forward link object.
    class CVampListEntry *Flink;

    // @cmember Backward link object.
    class CVampListEntry *Blink;
};

typedef CVampListEntry VAMP_LIST_ENTRY;     //@defitem VAMP_LIST_ENTRY | Vamp list entry
typedef VAMP_LIST_ENTRY *PVAMP_LIST_ENTRY;  //@defitem *PVAMP_LIST_ENTRY | Vamp list entry pointer


//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampList | Double-linked list access methods.
// @end
//////////////////////////////////////////////////////////////////////////////
template <class Object> class CVampList
{
    // @access private 
private:
    // @cmember Checks whether an element exists in the list.<nl>
    //Parameterlist:<nl>
    //PVAMP_LIST_ENTRY pnode // list entry pointer<nl>
    BOOL _ExistsInList( 
        PVAMP_LIST_ENTRY pnode )
    {
        DBGPRINT((1,"_ExistsInList( 0x%08x )\n", pnode));
        if( ( pnode == NULL ) ||
            ( pnode->Flink == NULL ) ||
            ( pnode->Blink == NULL ) )
        {
            DBGPRINT((1,"_ExistsInList() returns FALSE\n"));
            return FALSE;
        }

        for( VAMP_LIST_ENTRY * pItem = m_head.Flink;
                               pItem != &m_head; 
                               pItem = pItem->Flink )
        {
             if( pItem == pnode )
             {
                 DBGPRINT((1,"_ExistsInList() returns TRUE\n"));
                 return TRUE;
             }
        }

        DBGPRINT((1,"_ExistsInList() returns FALSE\n"));
        return FALSE;
    };

    // @cmember Removes a list entry.<nl>
    //Parameterlist:<nl>
    //PVAMP_LIST_ENTRY pnode // list entry pointer<nl>
    VOID _RemoveEntryList( 
        PVAMP_LIST_ENTRY pnode )
    {
        VAMP_LIST_ENTRY* pback = pnode->Blink;
        pback->Flink = pnode->Flink;
        pnode->Flink->Blink = pback;
        pnode->Flink = pnode->Blink = NULL;
    };

    // @cmember Returns an object pointer.<nl>
    //Parameterlist:<nl>
    //PVAMP_LIST_ENTRY pnode // list entry pointer<nl>
    Object *ToObject( 
        PVAMP_LIST_ENTRY pNode )
    {
        if( !pNode || ( pNode == &m_head ) )
        {
            DBGPRINT((1,"ToObject() returns NULL\n"));
            return NULL;
        }
        else
        {
            DBGPRINT((1,"ToObject() returns 0x%08x\n", pNode));
            return (Object*) pNode;
        }
    }

    // @access protected 
protected:
    // @cmember Head element.
    VAMP_LIST_ENTRY m_head;

    // @access public 
public:
    // @cmember Constructor.
    CVampList()
    {
        m_head.Flink = m_head.Blink = &m_head;
    }

    // @cmember Inserts an object at the head of the list.<nl>
    //Parameterlist:<nl>
    //Object *p // pointer to object<nl>
    void InsertHead( 
        Object *p )
    {
        DBGPRINT((1,"InsertHead( 0x%08x )\n", p));
        if( !p )
        {
            DBGBREAK(0);
            return;
        }
        if( !m_head.Blink )
        {
            DBGBREAK(0);
            return;
        }
        if( _ExistsInList( p ) )
        {
            DBGBREAK(0);
            return;
        }

        p->Flink = m_head.Flink;
        p->Blink = &m_head;
        m_head.Flink->Blink = p;
        m_head.Flink = p;
    }

    // @cmember Inserts an object at the tail of the list.<nl>
    //Parameterlist:<nl>
    //Object *p // pointer to object<nl>
    void InsertTail( 
        Object* p )
    {
        DBGPRINT((1,"InsertTail( 0x%08x )\n", p));
        if( !p )
        {
            DBGBREAK(0);
            return;
        }
        if( !m_head.Blink )
        {
            DBGBREAK(0);
            return;
        }
        if( _ExistsInList( p ) )
        {
            DBGBREAK(0);
            return;
        }

        p->Flink = &m_head;
        p->Blink = m_head.Blink;
        m_head.Blink->Flink = p;
        m_head.Blink = p;
    }

    // @cmember Removes an object (element) from the list.<nl>
    //Parameterlist:<nl>
    //Object *p // pointer to object<nl>
    Object *Remove( 
        Object* p )
    {
        DBGPRINT((1,"Remove( 0x%08x )\n", p));
        DBGBREAK(1);

        if( !_ExistsInList( p ) )
            return NULL;

        _RemoveEntryList( p );

        return p;
    }

    // @cmember Removes an object (element) from the head of the list.
    Object *RemoveHead()
    {
        DBGPRINT((1,"RemoveHead()\n"));
        if( IsEmpty() )
        {
            DBGPRINT((1,"RemoveHead() returns NULL\n"));
            return NULL;
        }

        PVAMP_LIST_ENTRY ple = m_head.Flink;
        _RemoveEntryList( ple );

        DBGPRINT((1,"RemoveHead() returns 0x%08x\n", ple));
        return ToObject( ple );
    }
    
    // @cmember Removes an object (element) from the tail of the list.
    Object *RemoveTail()
    {
        DBGPRINT((1,"RemoveTail()\n"));
        if( IsEmpty() )
        {
            DBGPRINT((1,"RemoveTail() returns NULL\n"));
            return NULL;
        }

        VAMP_LIST_ENTRY* ple = m_head.Blink;
        _RemoveEntryList( ple );

        DBGPRINT((1,"RemoveTail() returns 0x%08x\n", ple));
        return ToObject(ple);
    }

    // @cmember Returns the head object (element).
    Object *Head()
    {
        DBGPRINT((1,"Head()\n"));
        return ToObject( m_head.Flink );
    }
    
    // @cmember Returns the tail object (element).
    Object *Tail()
    {
        DBGPRINT((1,"Tail()\n"));
        return ToObject( m_head.Blink );
    }

   
    // @cmember Returns the next object (element).<nl>
    //Parameterlist:<nl>
    //Object *p // pointer to object<nl>
    Object *Next( 
        Object* p )
    {
        DBGPRINT((1,"Next( 0x%08x )\n", p));
        if( !p )
        {
            return Head();
        }
        return ToObject( p->Flink );
    }
    
    // @cmember Returns the previous object (element).<nl>
    //Parameterlist:<nl>
    //Object *p // pointer to object<nl>
    Object* Prev( 
        Object* p )
    {
        DBGPRINT((1,"Prev( 0x%08x )\n", p));
        if( !p )
        {
            return Tail();
        }
        return ToObject( p->Blink );
    }

    // @cmember Checks if the list is empty.
    bool IsEmpty()
    {
        DBGPRINT((1,"IsEmpty() returns %s\n", (m_head.Flink == &m_head) ? "TRUE" : "FALSE"));
        return (bool)( m_head.Flink == &m_head );
    }

    // @cmember Returns the number of list elements.
    int Count()
    {
        DBGPRINT((1,"Count()\n"));
        int cItems = 0;
        for( VAMP_LIST_ENTRY * pItem = m_head.Flink;
                               pItem != &m_head; 
                               pItem = pItem->Flink )
        {
            cItems++;
        }
        DBGPRINT((1,"... Count() returns %d\n", cItems));
        return cItems;
    }
};

#endif // _VAMPLIST_H_

