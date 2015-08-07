#include <stdio.h>
#include <my_global.h>
#include <mysql.h>

int main(int argc, char **argv)
{  
  MYSQL *con = mysql_init(NULL);

  if (con == NULL) 
  {
      fprintf(stderr, "%s\n", mysql_error(con));
      exit(1);
  }

  if (mysql_real_connect(con, "localhost", "root", "string", 
          NULL, 0, NULL, 0) == NULL) 
  {
      fprintf(stderr, "%s\n", mysql_error(con));
      mysql_close(con);
      exit(1);
  }  

  if (mysql_query(con, "CREATE DATABASE work")) 
  {
      //fprintf(stderr, "%s\n", mysql_error(con));
      //mysql_close(con);
      //exit(1);
      if (mysql_query(con,"Use work"))
      {
          //mysql_query();
      }
  }

  mysql_close(con);
  exit(0);
}
