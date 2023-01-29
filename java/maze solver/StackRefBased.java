public class StackRefBased<T> implements Stack<T> {
	int size;
	StackNode<T> top;
    public StackRefBased() {
    	size=0;
    	top=null;
    }

    public int size() {
        return size;
    }


    public boolean isEmpty() {
    	if(size==0)return true;
        return false;
    }


    public void push(T data) {
    	StackNode<T> newNode= new StackNode<T>(data);
    	newNode.next=top;
    	top=newNode;
    	size++;
    }


    public T pop() throws StackEmptyException {
        StackNode<T> oldTop = top;
        if(size==0)throw new StackEmptyException();
        top=top.getNext();
        size--;
    	return oldTop.getValue();
    }


    public T peek() throws StackEmptyException {
       
    	if(size==0)return null;
    	
    	return top.getValue();
    }


    public void makeEmpty() {
    	size=0;
    	top=null;
    }


    public String toString() {
    	StackNode<T> finder;
    	StackNode<T> last=null;
    	finder=this.top;
    	String s="";
        if(finder==null) return "Sweden!";
        
        else { while(last!=top){
        	
        	while(finder.next!=last) {
        		finder=finder.next;
        	}
        	last=finder;
        	s+=last.getValue();
        	if(finder!=top)s+=" ";
        	
        	finder=top;
        	
        	
        }
        }
        return s;
    }
}

