--------------------------------------------------------
--  File created - Thursday-April-28-2011   
--------------------------------------------------------
--------------------------------------------------------
--  DDL for Trigger T_INSERT_DATASET_VIEW
--------------------------------------------------------

  CREATE OR REPLACE TRIGGER "DEZHAO"."T_INSERT_DATASET_VIEW" 
INSTEAD OF INSERT ON DATASET_VIEW 
declare
  datasetid number;
  
  type_list type_list_t;
  contributor_list contributor_list_t;
  contributor contributor_t;
  
  copyright_list copyright_list_t;
  
  asso_dataset_list associating_dataset_list_t;
  asso_dataset associating_dataset_t;
  assoDatasetId number;
  
  v_count number;
BEGIN
  datasetid := :new.id;
  
  v_count := 0;
  SELECT count(*) INTO v_count FROM dataset WHERE id = datasetid;
  if(v_count = 0) then
    insert into data_item (id, description, flag) values (datasetid, :new.description, 'dataset');
    insert into image_data_item (id) values (datasetid);
    insert into logical_image_data_item (id) values (datasetid);
    insert into dataset (id, name, purpose) values (datasetid, :new.name, :new.purpose);
  else
    if(:new.name is not null) then
      update dataset set name=:new.name where id=datasetid;
    end if;
    if(:new.description is not null) then
      update data_item set description=:new.description where id=datasetid;
    end if;
  end if;
  
  --manage the datatypes of this dataset
  type_list := :new.type_list;
  if(type_list is not null and type_list.last>0) then
    for i in type_list.first .. type_list.last loop
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
          values (datasetid, type_list(i).id);
    end loop;
  end if;
  
  --manage the metadata about a page image's contributor list
  contributor_list := :new.contributor_list;
  if(contributor_list is not null and contributor_list.last>0) then
    for i in contributor_list.first .. contributor_list.last loop
      contributor := contributor_list(i);
      insert into contribute (person_id, data_item_id, description, type) 
        values (contributor.person_id, datasetid, contributor.description, contributor.type);
    end loop; --the end of going through all datasets
  end if; --the end of managing page image's contributor list
  
  --manage the copyright information of each dataset
  copyright_list := :new.copyright_list;
  if(copyright_list is not null and copyright_list.last>0) then
    for i in copyright_list.first .. copyright_list.last loop
      insert into has_copyright (data_item_id, file_id) 
        values (datasetid, copyright_list(i).id);
    end loop;
  end if;
  
  --start to manage the relationships between this dataset and all its associating datasets
  asso_dataset_list := :new.associating_dataset_list;
  if(asso_dataset_list is not null and asso_dataset_list.last>0) then
    for j in asso_dataset_list.first .. asso_dataset_list.last loop
      --get the jth associating dataset to the current dataset
        asso_dataset := asso_dataset_list(j);
        assoDatasetId := asso_dataset.id;
        insert into data_item (id, description, copyright, status) values (assoDatasetId, '', '', 0) 
          LOG ERRORS INTO errorlog ('ORA_ERR_MESG$') REJECT LIMIT UNLIMITED;
        insert into image_data_item (id) values (assoDatasetId) 
          LOG ERRORS INTO errorlog ('ORA_ERR_MESG$') REJECT LIMIT UNLIMITED;
        insert into logical_image_data_item (id) values (assoDatasetId) 
          LOG ERRORS INTO errorlog ('ORA_ERR_MESG$') REJECT LIMIT UNLIMITED;
        insert into dataset (id, name, purpose) values (assoDatasetId, '', '') 
          LOG ERRORS INTO errorlog ('ORA_ERR_MESG$') REJECT LIMIT UNLIMITED;
        
        --insert a record, meaning that these two datasets are associated with certain type
        insert into associate_dataset (dataset_id, associating_dataset_id, type) 
          values (datasetid, assoDatasetId, asso_dataset.type);
    end loop; --the end of going through the associating dataset list of one dataset
  end if; --the end of managing the associated dataset list
END;
/
ALTER TRIGGER "DEZHAO"."T_INSERT_DATASET_VIEW" ENABLE;
