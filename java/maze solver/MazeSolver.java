/*
 * MazeSolver.java
 *
 * UVic CSC 115, Spring 2017
 *
 * Purpose:
 *   class that contains a single public static method called
 *   "findPath". To involve the method one must have already created
 *   an instance of Maze. The method must return a path through the
 *   maze (if it exists) in the format shown within the Assignment #3
 *   description.
 *
 * Note: You are free to add to this class whatever other methods will
 * help you in writing a solution to A#3 part 2.
 */

public class MazeSolver {
    public static String findPath(Maze maze) {
        String result = "";
        
        boolean[][] locations=new boolean[maze.getNumRows()][maze.getNumCols()];
        
        Stack<MazeLocation> stack;
        stack=new StackRefBased<MazeLocation>();

        MazeLocation current=maze.getEntry();
     
        stack.push(current);
        
        while(stack.isEmpty()!=true && current.equals(maze.getExit())==false) {
       
        	
         	try {
				current=stack.peek();
			} catch (StackEmptyException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			}
        	locations[current.getRow()][current.getCol()]=true;
         
        	
        
        	//tests for walls/visited locations
        	
        	 if(maze.isWall(current.getRow()-1,current.getCol())==false && locations[current.getRow()-1][current.getCol()]==false) {
        		current= new MazeLocation(current.getRow()-1,current.getCol());
        		stack.push(current);
        	
        		continue;
        	}
        	
        	else if(maze.isWall(current.getRow(),current.getCol()-1)==false && locations[current.getRow()][current.getCol()-1]==false) {
        		current= new MazeLocation(current.getRow(),current.getCol()-1);
        		stack.push(current);
        		continue;
        	}
        	
        	else if(maze.isWall(current.getRow()+1,current.getCol())==false && locations[current.getRow()+1][current.getCol()]==false) {
        		
        		current= new MazeLocation(current.getRow()+1,current.getCol());
        		stack.push(current);
        		continue;
        		
        	}
        	else if(maze.isWall(current.getRow(),current.getCol()+1)==false && locations[current.getRow()][current.getCol()+1]==false) {
        		current= new MazeLocation(current.getRow(),current.getCol()+1);
        		stack.push(current);
        		continue;
        	}
        	
    
        	else {
        		
        			try {
        			
        		        	stack.pop();
				
					}
        			catch (StackEmptyException e) {
						// TODO Auto-generated catch block
						System.out.println("Oops lol");
						
					}
        		}
        }
        
        
        if(stack.isEmpty()) result="";
    	
    	else result=stack.toString();
    
        return result;
    }
}
