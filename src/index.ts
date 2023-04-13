import { Cluster } from "@nodepp/example";

const cluster = new Cluster({
  nodes: ["172.21.0.2"],
});

async function main() {
  const session = cluster.connect();
  const keyspaceName = session.executeSync(
    "SELECT keyspace_name FROM system_schema.keyspaces"
  );
  console.log(keyspaceName);
}
main();
