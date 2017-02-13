--------------------------------------------------------
--  File created - Thursday-April-28-2011   
--------------------------------------------------------
--------------------------------------------------------
--  DDL for Trigger ALGORITHM_INOUT_VIEW_INSERT
--------------------------------------------------------

  CREATE OR REPLACE TRIGGER "DEZHAO"."ALGORITHM_INOUT_VIEW_INSERT" 
   INSTEAD OF INSERT ON algorithm_inout_view
  declare
   algoID number;
   dataID number;
   paramName varchar2(2000);
   BEGIN
   --if we see a row that has a name element, then that is a new algorithm.
   if(:new.name is not null) then
   --insert all information about that algorithm
   INSERT INTO algorithm (id, name, description, path, version) 
     VALUES (:new.id, :new.name, :new.description, :new.path, :new.version);
     --if we see a row that has an inputname, then this row defines an input data
     --for the algorithm that we just inserted.
     else if (:new.inputname is not null) then
     if(:new.inputname = '-') then
     paramName := NULL;
     else
     paramName := :new.inputname;
     end if;
     insert into data (id, name, description) values (:new.id, paramName, :new.inputdescription);
     --we need to add the datatype information of this input data record
     select seq_data.currval into dataID from dual;
     insert into type_of(data_id, datatype_id) values (dataID, :new.inputdatatypeid);
     --after having inserted a new input data record, we need to make a relationship between 
     --the algorithm and this input data record.
   select seq_algorithm.currval into algoID from dual;   
   INSERT INTO algorithm_input (algorithm_id, data_id) VALUES (algoID, dataID);
   --if we see a row that has an outputname, then this row defines an output data
   --for the algorithm that we just inserted.
   else if (:new.outputname is not null) then
   if(:new.outputname = '-') then
     paramName := NULL;
     else
     paramName := :new.outputname;
     end if;
   --insert this output data record
   insert into data (id, name, description) values (:new.id, paramName, :new.outputdescription);
   --we need to add the datatype information of this output data record
    select seq_data.currval into dataID from dual;
     insert into type_of(data_id, datatype_id) values (dataID, :new.outputdatatypeid);
   --again, we need to connect the algorithm to this output data record.    
   select seq_algorithm.currval into algoID from dual;
   INSERT INTO algorithm_output (algorithm_id, data_id) VALUES (algoID, dataID);
   end if;
    end if;
     end if;
   END algorithm_inout_view_insert;
/
ALTER TRIGGER "DEZHAO"."ALGORITHM_INOUT_VIEW_INSERT" ENABLE;
