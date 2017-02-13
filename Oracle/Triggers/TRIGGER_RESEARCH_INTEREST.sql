create or replace
trigger trigger_research_interest
before insert on research_interest
for each row
begin
select seq_research_interest.nextval into :new.id from dual;
end;