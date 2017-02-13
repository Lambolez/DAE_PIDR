create or replace
TRIGGER TRIGGER_ALGORITHM_VIEW_INSERT
  INSTEAD OF INSERT ON ALGORITHM_VIEW
  declare
   TYPE population_type IS TABLE OF NUMBER index by varchar2(200);
   country population_type;
   algoID number;
   dataID number;
   list1 input_list;
   list2 output_list;
   item param;
   type_list datatype_list;
   datatype_item datatype_t;
   str varchar2(2000);
   temp varchar2(200);
   BEGIN
   str := :new.ALGORITHM_ID;
   insert into algorithm (id, description) 
      values (seq_algorithm.nextval, str);
   if (is_numeric(str) = 0) then
    country(str) := seq_algorithm.nextval;
    algoID := country(str);
   else
    algoID := is_numeric(str);
   end if;
   --insert a record into the algorithm table first
   if(country.exists('test') = false) then
    temp := 'test1';
   end if;
   INSERT INTO algorithm (id, name, path) 
    VALUES (algoID, :new.algorithm_name, temp);
   --store the algorithm ID into a variable
   --now, start to parse the input parameters of an algorithm and insert into records into the data table; 
   --also, after inserting one data record, we insert a record into the algorithm_input table to link this 
   --algorithm and its input parameters
   if (:new.in_list is not null) then
    --use a variable to store all the input parameters passed from the XML file
    list1 := :new.in_list;
    --go through each input parameter
    FOR i IN list1.FIRST .. list1.LAST
      LOOP
        --use a variable to store the ith input parameter
        item := list1(i);
        --insert the data record
        insert into data (name, description) 
          values (item.name, item.description);
        --put the data record ID into a variable
        dataID := seq_data.currval;
        --insert the record to link this algorithm and this input parameter
        insert into algorithm_input (algorithm_id, data_id)
          values (algoID, dataID);
        --finally, we need to link this input parameter to its datatype(s)
        if(item.type_list is not null) then
          type_list := item.type_list;
          for j in type_list.first .. type_list.last
            loop
              --get each datatype for this input parameter
              datatype_item := type_list(j);
              insert into type_of (data_id, datatype_id)
                values (dataID, datatype_item.datatype_id);
            end loop;
        end if;
      END LOOP;
   end if;
   --start to parse algorithm output parameters in the same way
   if (:new.out_list is not null) then
    --use a variable to store all the output parameters passed from the XML file
    list2 := :new.out_list;
    --go through each output parameter
    FOR i IN list2.FIRST .. list2.LAST
      LOOP
        --use a variable to store the ith output parameter
        item := list2(i);
        --insert the data record
        insert into data (name, description) 
          values (item.name, item.description);
        --put the data record ID into a variable
        dataID := seq_data.currval;
        --insert the record to link this algorithm and this output parameter
        insert into algorithm_output (algorithm_id, data_id)
          values (algoID, dataID);
        --finally, we need to link this output parameter to its datatype(s)
        if(item.type_list is not null) then
          type_list := item.type_list;
          for j in type_list.first .. type_list.last
            loop
              --get each datatype for this output parameter
              datatype_item := type_list(j);
              insert into type_of (data_id, datatype_id)
                values (dataID, datatype_item.datatype_id);
            end loop;
        end if;
      END LOOP;
   end if;
   END;