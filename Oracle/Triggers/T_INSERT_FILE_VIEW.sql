--------------------------------------------------------
--  File created - Thursday-April-28-2011   
--------------------------------------------------------
--------------------------------------------------------
--  DDL for Trigger T_INSERT_FILE_VIEW
--------------------------------------------------------

  CREATE OR REPLACE TRIGGER "DEZHAO"."T_INSERT_FILE_VIEW" 
INSTEAD OF INSERT ON FILE_VIEW 
declare
  fileid number;
  
  type_list type_list_t;
  copyright_list copyright_list_t;
  contributor_list contributor_list_t;
  contributor contributor_t;
  
  datasetID number;
  dataset dataset_t;
  dataset_list dataset_list_t;
  
  v_count number;
BEGIN
  fileid := :new.id;  
  insert into data_item (id, description, flag) values (fileid, :new.description, 'files');
  insert into files (id, name, path) values (fileid, :new.name, :new.path);
  
  --manage the datatypes of this file
  type_list := :new.type_list;
  if(type_list is not null and type_list.last>0) then
    for i in type_list.first .. type_list.last loop
      --this datatype might have been seen before, need to check before inserting
      v_count := 0;
      SELECT count(*) INTO v_count FROM datatype WHERE id = type_list(i).id;
      if(v_count = 0) then
        insert into datatype (id, name, description) 
          values (type_list(i).id, type_list(i).name, type_list(i).description);
      else
        if(type_list(i).name is not null) then
          update datatype set name=type_list(i).name where id=type_list(i).id;
        end if;
        if(type_list(i).description is not null) then
          update datatype set description=type_list(i).description where id=type_list(i).id;
        end if;
      end if;
       insert into associate_datatype_data_item (data_item_id, datatype_id) 
          values (fileid, type_list(i).id);
    end loop;
  end if;
  
  --manage the metadata about a page image's contributor list
  contributor_list := :new.contributor_list;
  if(contributor_list is not null and contributor_list.last>0) then
    for i in contributor_list.first .. contributor_list.last loop
      contributor := contributor_list(i);
      insert into contribute (person_id, data_item_id, description, type) 
        values (contributor.person_id, fileid, contributor.description, contributor.type);
    end loop; --the end of going through all datasets
  end if; --the end of managing page image's contributor list
  
  --manage the copyright information of each page image
  copyright_list := :new.copyright_list;
  if(copyright_list is not null and copyright_list.last>0) then
    for i in copyright_list.first .. copyright_list.last loop
      insert into has_copyright (data_item_id, file_id) 
        values (fileid, copyright_list(i).id);
    end loop;
  end if;
  
  --manage the metadata about the relationships between this file and the datasets that include it
  if(:new.dataset_list is not null and :new.dataset_list.last > 0) then
    --store the current image_dataset_list into a variable for easy reference
    dataset_list := :new.dataset_list;
    for i in dataset_list.first .. dataset_list.last loop
      dataset := dataset_list(i);
      datasetID := dataset.id;
      
      --insert a record describing the including relationship
      insert into includes_file (dataset_id, file_id) values (datasetID, fileid);
    end loop; --the end of going through all datasets
  end if; --the end of managing page image and its datasets
--EXCEPTION
--when others then
--declare 
--error_code NUMBER := SQLCODE;
--begin
--if (error_code = -2291) then
--  insert into error (datasetid, id) values (0, :new.id);
--end if;
--end;
END;
/
ALTER TRIGGER "DEZHAO"."T_INSERT_FILE_VIEW" ENABLE;
